/*#io
docCopyright("Steve Dekorte", 2004)
docLicense("BSD revised")
docObject("JFile")    
docDescription("""
  JFile - a journaled file
 
  How it works:
    1. writes are made to a log file (file.log) 
    2. when commit is called, a JFILE_COMMITTED value and the length of the log is written to the log header
       2.1 then the log writes are performed on output file 
       2.2 then log file header is set to JFILE_UNCOMMITTED
    3. when JFile is opened, if it finds:
       3.1 JFILE_UNCOMMITTED set in the log header
          it assumes the log is incomplete and overwrites it
       3.2 JFILE_COMMITTED set in the log header
          it goes to step 2.1
	  
  In this way, the output file is assured to be kept in a consistent state.
""")
*/

#ifndef JFILE_DEFINED
#define JFILE_DEFINED 1

#include <stdio.h>
#include <sys/types.h> 

#ifdef __cplusplus
extern "C" {
#endif

/*
// expiremental
#define JFILE_SUPPORTS_MMAP 1
*/

#define JFILE_UNCOMMITTED 0
#define JFILE_COMMITTED   1

typedef struct
{    
    char *path;
    FILE *file;

    char *logPath;
    FILE *log;
	
	long pos;
	long maxPos;
 
    unsigned char *buf;
	int journalingOn;
	int fullSync;
	size_t logHighWaterMark;
} JFile;

JFile *JFile_new(void);
void JFile_free(JFile *self);

void JFile_setJournalingOn_(JFile *self, int aBool);

void JFile_setPath_(JFile *self, const char *path);
void JFile_setLogPath_(JFile *self, const char *path);
void JFile_setPath_withExtension_(JFile *self, const char *path, const char *ext);
char *JFile_path(JFile *self);

void JFile_open(JFile *self);
void JFile_close(JFile *self);
void JFile_delete(JFile *self);

size_t JFile_fwrite(JFile *self, void *buf, size_t size, size_t nobjs);
size_t JFile_fread(JFile *self, void *buf, size_t size, size_t nobjs);

int JFile_fputc(JFile *self, int c);
int JFile_fgetc(JFile *self);

void JFile_writeInt_(JFile *self, int v);
int JFile_readInt(JFile *self);

void JFile_setPosition_(JFile *self, long pos);
long JFile_setPositionToEnd(JFile *self);
void JFile_fseek(JFile *self, long offset, int whence);
long JFile_position(JFile *self);

void JFile_begin(JFile *self);
void JFile_commitToLog(JFile *self); // makes sure log is on disk
void JFile_commitToFile(JFile *self);   // writes log to file and makes sure changes are on disk
void JFile_remove(JFile *self);

void JFile_truncate_(JFile *self, off_t size);

void JFile_setTrueFlush_(JFile *self, int aBool);
size_t JFile_logSize(JFile *self);
int JFile_needsSync(JFile *self);
long JFile_lastCommitPosition(JFile *self);
void JFile_clipLogToLastCompletedTransaction(JFile *self);
void JFile_clipLog(JFile *self);
//int JFile_hasUnfinishedCommits(JFile *self);
int JFile_verifyLog(JFile *self);

#ifdef __cplusplus
}
#endif
#endif
