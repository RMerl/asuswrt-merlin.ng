//=========================================================================
// FILENAME	: tagutils-dsf.c
// DESCRIPTION	: DSF metadata reader
//=========================================================================
// Copyright (c) 2014 Takeshich NAKAMURA
// based on tagutils-mp3.c
//=========================================================================

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define GET_DSF_INT64(p) ((((uint64_t)((p)[7])) << 56) |   \
			  (((uint64_t)((p)[6])) << 48) |   \
			  (((uint64_t)((p)[5])) << 40) |   \
			  (((uint64_t)((p)[4])) << 32) |   \
			  (((uint64_t)((p)[3])) << 24) |   \
			  (((uint64_t)((p)[2])) << 16) |   \
			  (((uint64_t)((p)[1])) << 8) |    \
			  (((uint64_t)((p)[0]))))

#define GET_DSF_INT32(p) ((((uint8_t)((p)[3])) << 24) |   \
			  (((uint8_t)((p)[2])) << 16) |   \
			  (((uint8_t)((p)[1])) << 8) |     \
			  (((uint8_t)((p)[0]))))

static int
_get_dsftags(char *file, struct song_metadata *psong)
{
	struct id3_tag *pid3tag;
	struct id3_frame *pid3frame;
	int err;
	int index;
	int used;
	unsigned char *utf8_text;
	int genre = WINAMP_GENRE_UNKNOWN;
	int have_utf8;
	int have_text;
	id3_ucs4_t const *native_text;
	char *tmp;
	int got_numeric_genre;
	id3_byte_t const *image;
	id3_length_t image_size = 0;

	FILE *fp;
	struct id3header *pid3;
	uint32_t len;
	unsigned char hdr[28] = { 0 };
	uint64_t total_size = 0;
	uint64_t pointer_to_metadata_chunk = 0;
	uint64_t metadata_chunk_size = 0;
	unsigned char *id3tagbuf = NULL;

	//DEBUG DPRINTF(E_DEBUG,L_SCANNER,"Getting DSF file info\n");

	if ((fp = fopen(file, "rb")) == NULL)
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not create file handle\n");
		return -1;
	}

	len = 28;
	if (!(len = fread(hdr, len, 1, fp)))
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not read DSD Chunk from %s\n", file);
		fclose(fp);
		return -1;
	}

	if (strncmp((char*)hdr, "DSD ", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid DSD Chunk header in %s\n", file);
		fclose(fp);
		return -1;
	}

	total_size = GET_DSF_INT64(hdr + 12);
	pointer_to_metadata_chunk = GET_DSF_INT64(hdr + 20);

	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "%llu\n", total_size);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "%llu\n", pointer_to_metadata_chunk);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "%llu\n", metadata_chunk_size);

	//check invalid metadata
	if (total_size == 0)
	{
		fclose(fp);
		DPRINTF(E_INFO, L_SCANNER, "Invalid TotalDataSize in %s\n", file);
		return 0;
	}

	if (pointer_to_metadata_chunk == 0)
	{
		fclose(fp);
		DPRINTF(E_INFO, L_SCANNER, "Metadata doesn't exist %s\n", file);
		return 0;
	}

	if (total_size > pointer_to_metadata_chunk)
	{
		metadata_chunk_size = total_size - pointer_to_metadata_chunk;
	}
	else
	{
		fclose(fp);
		DPRINTF(E_INFO, L_SCANNER, "Invalid PointerToMetadata in %s\n", file);
		return 0;
	}

	fseeko(fp, pointer_to_metadata_chunk, SEEK_SET);

	id3tagbuf = (unsigned char*)malloc(sizeof(unsigned char) * metadata_chunk_size);
	if (id3tagbuf == NULL)
	{
		fclose(fp);
		DPRINTF(E_WARN, L_SCANNER, "Out of memory.Big MetadataSize in %s\n", file);
		return -1;
	}
	memset(id3tagbuf, 0, sizeof(unsigned char) * metadata_chunk_size);

	if (!(len = fread(id3tagbuf, metadata_chunk_size, 1, fp)))
	{
		fclose(fp);
		free(id3tagbuf);
		DPRINTF(E_WARN, L_SCANNER, "Could not read Metadata Chunk from %s\n", file);
		return -1;
	}

	pid3tag = id3_tag_parse(id3tagbuf, metadata_chunk_size);

	if (!pid3tag)
	{
		fclose(fp);
		free(id3tagbuf);
		err = errno;
		errno = err;
		DPRINTF(E_WARN, L_SCANNER, "Cannot get ID3 tag for %s\n", file);
		return -1;
	}

	pid3 = (struct id3header*)id3tagbuf;

	if (strncmp((char*)pid3->id, "ID3", 3) == 0)
	{
		char tagversion[16];

		/* found an ID3 header... */
		snprintf(tagversion, sizeof(tagversion), "ID3v2.%d.%d",
			 pid3->version[0], pid3->version[1]);
		psong->tagversion = strdup(tagversion);
	}
	pid3 = NULL;

	index = 0;
	while ((pid3frame = id3_tag_findframe(pid3tag, "", index)))
	{
		used = 0;
		utf8_text = NULL;
		native_text = NULL;
		have_utf8 = 0;
		have_text = 0;

		if (!strcmp(pid3frame->id, "YTCP"))   /* for id3v2.2 */
		{
			psong->compilation = 1;
			DPRINTF(E_DEBUG, L_SCANNER, "Compilation: %d [%s]\n", psong->compilation, basename(file));
		}
		else if (!strcmp(pid3frame->id, "APIC") && !image_size)
		{
			if ((strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "image/jpeg") == 0) ||
			    (strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "image/jpg") == 0) ||
			    (strcmp((char*)id3_field_getlatin1(&pid3frame->fields[1]), "jpeg") == 0))
			{
				image = id3_field_getbinarydata(&pid3frame->fields[4], &image_size);
				if (image_size)
				{
					psong->image = malloc(image_size);
					memcpy(psong->image, image, image_size);
					psong->image_size = image_size;
					//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "Found thumbnail: %d\n", psong->image_size);
				}
			}
		}

		if (((pid3frame->id[0] == 'T') || (strcmp(pid3frame->id, "COMM") == 0)) &&
		    (id3_field_getnstrings(&pid3frame->fields[1])))
			have_text = 1;

		if (have_text)
		{
			native_text = id3_field_getstrings(&pid3frame->fields[1], 0);

			if (native_text)
			{
				have_utf8 = 1;
				if (lang_index >= 0)
					utf8_text = _get_utf8_text(native_text); // through iconv
				else
					utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);

				if (!strcmp(pid3frame->id, "TIT2"))
				{
					used = 1;
					psong->title = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TPE1"))
				{
					used = 1;
					psong->contributor[ROLE_ARTIST] = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TALB"))
				{
					used = 1;
					psong->album = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TCOM"))
				{
					used = 1;
					psong->contributor[ROLE_COMPOSER] = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TIT1"))
				{
					used = 1;
					psong->grouping = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TPE2"))
				{
					used = 1;
					psong->contributor[ROLE_BAND] = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TPE3"))
				{
					used = 1;
					psong->contributor[ROLE_CONDUCTOR] = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TCON"))
				{
					used = 1;
					psong->genre = (char*)utf8_text;
					got_numeric_genre = 0;
					if (psong->genre)
					{
						if (!strlen(psong->genre))
						{
							genre = WINAMP_GENRE_UNKNOWN;
							got_numeric_genre = 1;
						}
						else if (isdigit(psong->genre[0]))
						{
							genre = atoi(psong->genre);
							got_numeric_genre = 1;
						}
						else if ((psong->genre[0] == '(') && (isdigit(psong->genre[1])))
						{
							genre = atoi((char*)&psong->genre[1]);
							got_numeric_genre = 1;
						}

						if (got_numeric_genre)
						{
							if ((genre < 0) || (genre > WINAMP_GENRE_UNKNOWN))
								genre = WINAMP_GENRE_UNKNOWN;
							free(psong->genre);
							psong->genre = strdup(winamp_genre[genre]);
						}
					}
				}
				else if (!strcmp(pid3frame->id, "COMM"))
				{
					used = 1;
					psong->comment = (char*)utf8_text;
				}
				else if (!strcmp(pid3frame->id, "TPOS"))
				{
					tmp = (char*)utf8_text;
					strsep(&tmp, "/");
					if (tmp)
					{
						psong->total_discs = atoi(tmp);
					}
					psong->disc = atoi((char*)utf8_text);
				}
				else if (!strcmp(pid3frame->id, "TRCK"))
				{
					tmp = (char*)utf8_text;
					strsep(&tmp, "/");
					if (tmp)
					{
						psong->total_tracks = atoi(tmp);
					}
					psong->track = atoi((char*)utf8_text);
				}
				else if (!strcmp(pid3frame->id, "TDRC"))
				{
					psong->year = atoi((char*)utf8_text);
				}
				else if (!strcmp(pid3frame->id, "TLEN"))
				{
					psong->song_length = atoi((char*)utf8_text);
				}
				else if (!strcmp(pid3frame->id, "TBPM"))
				{
					psong->bpm = atoi((char*)utf8_text);
				}
				else if (!strcmp(pid3frame->id, "TCMP"))
				{
					psong->compilation = (char)atoi((char*)utf8_text);
				}
			}
		}

		// check if text tag
		if ((!used) && (have_utf8) && (utf8_text))
			free(utf8_text);

		// v2 COMM
		if ((!strcmp(pid3frame->id, "COMM")) && (pid3frame->nfields == 4))
		{
			native_text = id3_field_getstring(&pid3frame->fields[2]);
			if (native_text)
			{
				utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);
				if ((utf8_text) && (strncasecmp((char*)utf8_text, "iTun", 4) != 0))
				{
					// read comment
					free(utf8_text);

					native_text = id3_field_getfullstring(&pid3frame->fields[3]);
					if (native_text)
					{
						utf8_text = (unsigned char*)id3_ucs4_utf8duplicate(native_text);
						if (utf8_text)
						{
							free(psong->comment);
							psong->comment = (char*)utf8_text;
						}
					}
				}
				else
				{
					free(utf8_text);
				}
			}
		}

		index++;
	}

	id3_tag_delete(pid3tag);
	free(id3tagbuf);
	fclose(fp);
	//DPRINTF(E_DEBUG, L_SCANNER, "Got id3tag successfully for file=%s\n", file);
	return 0;
}

static int
_get_dsffileinfo(char *file, struct song_metadata *psong)
{
	FILE *fp;
	int len = 80;
	unsigned char hdr[len];
	uint32_t channelnum;
	uint32_t samplingfrequency;
	uint32_t bitpersample;
	uint64_t samplecount;

	if ((fp = fopen(file, "rb")) == NULL)
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not create file handle\n");
		return -1;
	}

	if (!(len = fread(hdr, len, 1, fp)))
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not read chunks from %s\n", file);
		fclose(fp);
		return -1;
	}

	if (strncmp((char*)hdr, "DSD ", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid DSD Chunk headerin %s\n", file);
		fclose(fp);
		return -1;
	}

	if (strncmp((char*)hdr + 28, "fmt ", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid fmt Chunk header in %s\n", file);
		fclose(fp);
		return -1;
	}

	channelnum = GET_DSF_INT32(hdr + 52);
	samplingfrequency = GET_DSF_INT32(hdr + 56);
	bitpersample = GET_DSF_INT32(hdr + 60);
	samplecount = GET_DSF_INT64(hdr + 64);

	psong->bitrate = channelnum * samplingfrequency * bitpersample;
	psong->samplesize = bitpersample;
	psong->samplerate = samplingfrequency;
	psong->song_length = (samplecount / samplingfrequency) * 1000;
	psong->channels = channelnum;

	DPRINTF(E_MAXDEBUG, L_SCANNER, "Got file info successfully for %s\n", file);
	//DEBUG DPRINTF(E_MAXDEBUG, L_SCANNER, "bitrate is  %d\n", psong->bitrate);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "samplesize is  %d\n", psong->samplesize);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "samplerate is  %d\n", psong->samplerate);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "song_length is  %d\n", psong->song_length);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "channels are  %d\n", psong->channels);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "samplecount are  %lld\n", samplecount);
	fclose(fp);

	xasprintf(&(psong->dlna_pn), "DSF");
	return 0;
}
