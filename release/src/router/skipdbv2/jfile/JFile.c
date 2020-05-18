/*#io
JFile ioDoc(
		  docCopyright("Steve Dekorte", 2004)
		  docLicense("BSD revised")
		  docObject("JFile")
		  docDescription("A journaled file.")
		  log file is currently endian dependent
*/

#include "JFile.h"
#include "Common.h"	// for Win32 snprintf
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "PortableTruncate.h"

// See: http://lists.apple.com/archives/darwin-dev/2005/Feb/msg00072.html

#include <fcntl.h>

#define JFILE_END_TRANSACTION_ITEM 1
#define JFILE_END_TRANSACTION 2

void JFile_clipLog(JFile *self);
void JFile_writeLogToFile(JFile *self);
void JFile_flushLog(JFile *self);
void JFile_flushFile(JFile *self);

JFile *JFile_new(void)
{
	JFile *self = (JFile *)calloc(1, sizeof(JFile));
	//JFile_setPath_(self, "default");
	JFile_setJournalingOn_(self, 1);
	self->logHighWaterMark = 1024*1024*8;
	self->fullSync = 1;
	return self;
}

void JFile_setJournalingOn_(JFile *self, int aBool)
{
	self->journalingOn = aBool;
}

void JFile_free(JFile *self)
{
	JFile_close(self);
	if (self->path) free(self->path);
	if (self->logPath) free(self->logPath);
	if (self->buf) free(self->buf);
	free(self);
}

void JFile_setPath_(JFile *self, const char *path)
{
	self->path    = strcpy((char *)realloc(self->path,    strlen(path)+1), path);
	self->logPath = strcpy((char *)realloc(self->logPath, strlen(path)+5), path);
	strcat(self->logPath,  ".log");
}

char *JFile_fileName(JFile *self)
{
#ifdef __WIN32__
	char *fileName = strrchr(self->path, '\\');
#else
	char *fileName = strrchr(self->path, '/');
#endif
	return  fileName ? fileName : self->path;
}

void JFile_setLogPath_(JFile *self, const char *path)
{
	char *fileName = JFile_fileName(self);
	self->logPath = strcpy((char *)realloc(self->logPath, strlen(path) + strlen(fileName)+10), path);
	if (self->logPath[strlen(self->logPath)-1] != '/') strcat(self->logPath, "/");
	strcat(self->logPath, fileName);
	strcat(self->logPath,  ".log");
}

void JFile_setPath_withExtension_(JFile *self, const char *path, const char *ext)
{
	size_t length = strlen(path) + 1 + strlen(ext) + 1;
	char *s = (char *)calloc(1, length);
	strcat(s, path);
	strcat(s, ".");
	strcat(s, ext);
	JFile_setPath_(self, s);
	free(s);
}

char *JFile_path(JFile *self)
{
	return self->path;
}

void JFile_remove(JFile *self)
{
	JFile_close(self);
	remove(self->path);
	remove(self->logPath);
}

void JFile_open(JFile *self)
{
	self->file = fopen(self->path, "r+");

	if (!self->file)
	{
		// if it doesn't exist, create file
		self->file = fopen(self->path, "w");
		fclose(self->file);
		self->file = fopen(self->path, "r+");
	}

	self->pos = 0;
	fseek(self->file, 0, SEEK_END);
	self->maxPos = ftell(self->file);

	self->log = fopen(self->logPath, "r+");

	if (!self->log)
	{
		// if log doesn't exist, create it
		self->log = fopen(self->logPath, "w");
		fclose(self->log);
		self->log = fopen(self->logPath, "r+");
		#ifdef F_PREALLOCATE
			// OSX
			fcntl(fileno(self->log), F_PREALLOCATE, self->logHighWaterMark);
		#else
			// FIXME by jannson Linux
			//posix_fallocate(fileno(self->log), -1, self->logHighWaterMark);
		#endif
	}
	else
	{
		// if log exists, write any completed transactions to file
		JFile_clipLogToLastCompletedTransaction(self);
		JFile_writeLogToFile(self);
	}
}

void JFile_close(JFile *self)
{
	if (self->file)
	{
		fclose(self->file);
		self->file = NULL;
	}

	if (self->log)
	{
		fclose(self->log);
		self->log = NULL;
	}
}

void JFile_delete(JFile *self)
{
	JFile_close(self);
	remove(self->path);
	remove(self->logPath);
}

size_t JFile_fwrite(JFile *self, void *buf, size_t size, size_t nobjs)
{
	size_t total = size * nobjs;
	size_t nobjsWritten;

	if (self->journalingOn)
	{
		fwrite(&(self->pos), sizeof(long), 1, self->log);
		fwrite(&total, sizeof(size_t), 1, self->log);
		nobjsWritten = fwrite(buf, size, nobjs, self->log);
		fputc(JFILE_END_TRANSACTION_ITEM, self->log); // commit byte
		self->pos += total;
	}
	else
	{
		fseek(self->file, self->pos, SEEK_SET);
		nobjsWritten = fwrite(buf, size, nobjs, self->file);
		self->pos = ftell(self->file);
	}

	if (nobjs != nobjsWritten)
	{
		printf("Error: JFile_fwrite nobjs != nobjsWritten\n");
	}

	if (self->pos > self->maxPos) self->maxPos = self->pos;

	return nobjsWritten;
}

size_t JFile_fread(JFile *self, void *buf, size_t size, size_t nobjs)
{
	size_t nobjsRead;

	fseek(self->file, self->pos, SEEK_SET);
	nobjsRead = fread(buf, size, nobjs, self->file);
	self->pos = ftell(self->file);

	return nobjsRead;
}

int JFile_fputc(JFile *self, int i)
{
	unsigned char c = i;
	JFile_fwrite(self, &c, 1, 1);
	return i;
}

int JFile_fgetc(JFile *self)
{
	char *v;
	JFile_fread(self, &v, 1, 1);
	return (int)v;
}

void JFile_writeInt_(JFile *self, int v)
{
	JFile_fwrite(self, &v, sizeof(int), 1);
}

int JFile_readInt(JFile *self)
{
	int v;
	JFile_fread(self, &v, sizeof(int), 1);
	return v;
}

void JFile_setPosition_(JFile *self, long pos)
{
	self->pos = pos;
}

void JFile_fseek(JFile *self, long offset, int whence)
{
	switch (whence)
	{
		case SEEK_CUR:
			self->pos += offset;
			break;
		case SEEK_SET:
			self->pos = offset;
			break;
		case SEEK_END:
			self->pos = self->maxPos + offset;
			break;
	}
}

long JFile_position(JFile *self)
{
	return self->pos;
}

long JFile_setPositionToEnd(JFile *self)
{
	self->pos = self->maxPos;
	return self->pos;
}

void JFile_hardSyncFileDescriptor_(JFile *self, int fd)
{
	#ifdef F_FULLFSYNC
		fcntl(fd, F_FULLFSYNC, NULL);
	#else
		#warning Linux cant ensure data sync to physical media
		fsync(fd);
	#endif
}

void JFile_syncFileWritesToDisk(JFile *self)
{
	//printf("f"); fflush(stdout);
	fflush(self->file);

	if(self->fullSync)
	{
		JFile_hardSyncFileDescriptor_(self, fileno(self->file));
	}
}

void JFile_syncLogWritesToDisk(JFile *self)
{
	//printf("j"); fflush(stdout);
	fflush(self->log);
	if (self->fullSync)
	{
			JFile_hardSyncFileDescriptor_(self, fileno(self->log));
	}

	fseek(self->log, -1, SEEK_END);

	fputc(JFILE_END_TRANSACTION, self->log);

	fflush(self->log);
	if (self->fullSync)
	{
			JFile_hardSyncFileDescriptor_(self, fileno(self->log));
	}
}

void JFile_writeLogToFileIfNeeded(JFile *self)
{
	if (ftell(self->log))
	{
		JFile_writeLogToFile(self);
	}
}

void JFile_writeLogToFile(JFile *self)
{
	size_t writeCount = 0;
	int logNotEmpty = 0;
	int lastTerminator = 0;
	fseek(self->log, 0, SEEK_SET);

	while (1)
	{
		long pos;
		size_t total;

		if (fread(&pos, sizeof(long), 1, self->log) != 1)
		{
			if (feof(self->log)) break;
			printf("JFile: error reading pos\n");
			goto fatalError;
		}

		if (fread(&total, sizeof(size_t), 1, self->log) != 1)
		{
			printf("JFile: error reading total\n");
			goto fatalError;
		}

                //by janson, TODO better
                if (0 == total) {
                    return;
                }

		self->buf = realloc(self->buf, total);

		if (fread(self->buf, total, 1, self->log) != 1)
		{
			printf("JFile: error reading buf %d\n", total);
			goto fatalError;
		}

		lastTerminator = fgetc(self->log);

		if (lastTerminator != JFILE_END_TRANSACTION_ITEM &&
			lastTerminator != JFILE_END_TRANSACTION)
		{
			printf("JFile: invalid log terminator\n");
			goto fatalError;
		}

		fseek(self->file, pos, SEEK_SET);
		fwrite(self->buf, total, 1, self->file);
		writeCount ++;
		logNotEmpty = 1;
	}

	if (logNotEmpty)
	{
		//printf("writing log\n");
		JFile_syncFileWritesToDisk(self);
		JFile_clipLog(self);
	}

	return;

	fatalError:
		printf("JFile: FATAL ERROR: invalid log file '%s' - may result in inconsistency\n", self->logPath);

}


void JFile_begin(JFile *self)
{
	// begin is implicit unless we need to mark a rollback point
}

void JFile_commitToFile(JFile *self)
{
	JFile_commitToLog(self);
	/*
	if (!JFile_verifyLog(self))
	{
		printf("log '%s' not valid\n", self->logPath);
		exit(-1);
	}
	*/
	JFile_writeLogToFile(self);
}

void JFile_commitToLog(JFile *self)
{
	if (self->journalingOn)
	{
		if (ftell(self->log))
		{
			JFile_syncLogWritesToDisk(self);
		}
	}
	else
	{
		JFile_syncFileWritesToDisk(self);
	}
}

#include <unistd.h>

void JFile_clipLog(JFile *self)
{
	if (!self->log)
	{
		self->log = fopen(self->logPath, "r+");

		if (self->log)
		{
			ftruncate(fileno(self->log), 0);
			fseek(self->log, 0, SEEK_SET);
		}
	}
	else
	{
		ftruncate(fileno(self->log), 0);
		fseek(self->log, 0, SEEK_SET);
	}
}

void JFile_truncate_(JFile *self, off_t size)
{
	JFile_commitToLog(self);
	JFile_commitToFile(self);
	ftruncate(fileno(self->file), size);
}

int JFile_needsSync(JFile *self)
{
	return JFile_logSize(self) != 0;
}

size_t JFile_logSize(JFile *self)
{
	// this only valid when not in the middle of a commit
	return ftell(self->log);
}

long JFile_lastCommitPosition(JFile *self)
{
	long lastCommitPosition = 0;
	long end;
	fseek(self->log, 0, SEEK_END);
	end = ftell(self->log);
	fseek(self->log, 0, SEEK_SET);

	while (1)
	{
		long pos;
		size_t total;
		int r;

		r = fread(&pos, sizeof(long), 1, self->log);
		if (r != 1)
		{
			if (feof(self->log)) break;

			//printf("log broken at pos");
			break;
		}

		r = fread(&total, sizeof(size_t), 1, self->log);
		if (r != 1)
		{
			//printf("log broken at size");
			break;
		}

		if(fseek(self->log, total, SEEK_CUR) != 0)
		{
			//printf("log broken at data");
			break;
		}

		switch (fgetc(self->log))
		{
			case JFILE_END_TRANSACTION_ITEM:
				//printf("JFILE_END_TRANSACTION_ITEM at %i\n", ftell(self->log));
				continue;
			case JFILE_END_TRANSACTION:
				//printf("JFILE_END_TRANSACTION at %i\n", ftell(self->log));
				lastCommitPosition = ftell(self->log);
				continue;
			default:
				break;
		}
	}

	fseek(self->log, 0, SEEK_END);
	return lastCommitPosition;
}

int JFile_verifyLog(JFile *self)
{
	long lastCommitPosition = JFile_lastCommitPosition(self);

	fseek(self->log, 0, SEEK_END);

	return lastCommitPosition == ftell(self->log);
}

void JFile_clipLogToLastCompletedTransaction(JFile *self)
{
	long lastCommitPosition = JFile_lastCommitPosition(self);

	fseek(self->log, 0, SEEK_END);

	if (lastCommitPosition != ftell(self->log))
	{
		printf("JFile: incomplete transaction found in '%s' - discarding\n", self->logPath);
		ftruncate(fileno(self->log), lastCommitPosition);
		fseek(self->log, 0, SEEK_END);
	}
}

