//=========================================================================
// FILENAME     : tagutils-dff.c
// DESCRIPTION  : DFF metadata reader
//=========================================================================
// Copyright (c) 2014 Takeshich NAKAMURA
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
#define GET_DFF_INT64(p) ((((uint64_t)((p)[0])) << 56) |   \
			  (((uint64_t)((p)[1])) << 48) |   \
			  (((uint64_t)((p)[2])) << 40) |   \
			  (((uint64_t)((p)[3])) << 32) |   \
			  (((uint64_t)((p)[4])) << 24) |   \
			  (((uint64_t)((p)[5])) << 16) |   \
			  (((uint64_t)((p)[6])) << 8) |    \
			  (((uint64_t)((p)[7]))))

#define GET_DFF_INT32(p) ((((uint32_t)((p)[0])) << 24) |   \
			  (((uint32_t)((p)[1])) << 16) |   \
			  (((uint32_t)((p)[2])) << 8) |     \
			  (((uint32_t)((p)[3]))))

#define GET_DFF_INT16(p) ((((uint16_t)((p)[0])) << 8) |   \
			  (((uint16_t)((p)[1]))))

static int
_get_dfffileinfo(char *file, struct song_metadata *psong)
{
	FILE *fp;
	uint32_t len;
	uint32_t rt;
	unsigned char hdr[32] = { 0 };

	uint64_t totalsize = 0;
	uint64_t propckDataSize = 0;
	uint64_t count = 0;
	uint32_t samplerate = 0;
	uint16_t channels = 0;
	//DST
	uint64_t dstickDataSize = 0;
	uint32_t numFrames = 0;
	uint16_t frameRate = 0;
	unsigned char frteckData[18] = { 0 };
	unsigned char dstickData[12] = { 0 };
	uint64_t totalcount = 0;
	unsigned char ckbuf[12] = { 0 };
	unsigned char compressionType[4] = { 0 };
	unsigned char dsdsdckData[12] = { 0 };
	uint64_t dsdsdckDataSize = 0;
	uint64_t cmprckDataSize = 0;
	uint64_t abssckDataSize = 0;
	uint64_t lscockDataSize = 0;
	uint64_t comtckDataSize = 0;
	uint64_t diinckDataSize = 0;
	uint64_t diarckDataSize = 0;
	uint64_t ditickDataSize = 0;
	uint64_t manfckDataSize = 0;

	//DPRINTF(E_DEBUG,L_SCANNER,"Getting DFF fileinfo =%s\n",file);

	if ((fp = fopen(file, "rb")) == NULL)
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not create file handle\n");
		return -1;
	}

	len = 32;
	//Form DSD chunk
	if (!(rt = fread(hdr, len, 1, fp)))
	{
		DPRINTF(E_WARN, L_SCANNER, "Could not read Form DSD chunk from %s\n", file);
		fclose(fp);
		return -1;
	}

	if (strncmp((char*)hdr, "FRM8", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid Form DSD chunk in %s\n", file);
		fclose(fp);
		return -1;
	}

	totalsize = GET_DFF_INT64(hdr + 4);

	if (strncmp((char*)hdr + 12, "DSD ", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid Form DSD chunk in %s\n", file);
		fclose(fp);
		return -1;
	}

	//FVER chunk
	if (strncmp((char*)hdr + 16, "FVER", 4))
	{
		DPRINTF(E_WARN, L_SCANNER, "Invalid Format Version Chunk in %s\n", file);
		fclose(fp);
		return -1;
	}

	totalsize -= 16;
	while (totalcount < totalsize - 4)
	{
		if (!(rt = fread(ckbuf, sizeof(ckbuf), 1, fp)))
		{
			//DPRINTF(E_WARN, L_SCANNER, "Could not read chunk header from %s\n", file);
			//fclose(fp);
			//return -1;
			break;
		}

		//Property chunk
		if (strncmp((char*)ckbuf, "PROP", 4) == 0)
		{
			propckDataSize = GET_DFF_INT64(ckbuf + 4);
			totalcount += propckDataSize + 12;

			unsigned char propckData[propckDataSize];

			if (!(rt = fread(propckData, propckDataSize, 1, fp)))
			{
				DPRINTF(E_WARN, L_SCANNER, "Could not read Property chunk from %s\n", file);
				fclose(fp);
				return -1;
			}

			if (strncmp((char*)propckData, "SND ", 4))
			{
				DPRINTF(E_WARN, L_SCANNER, "Invalid Property chunk in %s\n", file);
				fclose(fp);
				return -1;
			}

			count += 4;
			while (count < propckDataSize)
			{
				if (strncmp((char*)propckData + count, "FS  ", 4) == 0)
				{
					//Sample Rate Chunk
					count += 12;
					samplerate = GET_DFF_INT32(propckData + count);
					psong->samplerate = samplerate;
					count += 4;

					//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "Sample Rate is %d\n", psong->samplerate);
				}
				else if (strncmp((char*)propckData + count, "CHNL", 4) == 0)
				{
					//Channels Chunk
					count += 12;
					channels = GET_DFF_INT16(propckData + count);
					psong->channels = channels;
					count += channels * 4 + 2;

					//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "channels is %d\n", channels);
				}
				else if (strncmp((char*)propckData + count, "CMPR", 4) == 0)
				{
					//Compression Type Chunk
					count += 4;
					cmprckDataSize = GET_DFF_INT64(propckData + count);
					count += 8;
					memcpy(compressionType, propckData + count, 4);
					count += cmprckDataSize;
				}
				else if (strncmp((char*)propckData + count, "ABSS", 4) == 0)
				{
					//Absolute Start Time Chunk
					count += 4;
					abssckDataSize = GET_DFF_INT64(propckData + count);
					count += abssckDataSize + 8;
				}
				else if (strncmp((char*)propckData + count, "LSCO", 4) == 0)
				{
					//Loudsperaker Configuration Chunk
					count += 4;
					lscockDataSize = GET_DFF_INT64(propckData + count);
					count += lscockDataSize + 8;
				}
				else
				{
					break;
				}
			}

			//bitrate bitpersample is 1bit
			psong->bitrate = channels * samplerate * 1;

			//DSD/DST Sound Data Chunk
			len = 12;
			if (!(rt = fread(dsdsdckData, len, 1, fp)))
			{
				DPRINTF(E_WARN, L_SCANNER, "Could not read DSD/DST Sound Data chunk from %s\n", file);
				fclose(fp);
				return -1;
			}

			if (strncmp((char*)compressionType, (char*)dsdsdckData, 4))
			{
				DPRINTF(E_WARN, L_SCANNER, "Invalid DSD/DST Sound Data chunk in %s\n", file);
				fclose(fp);
				return -1;
			}

			if (strncmp((char*)dsdsdckData, "DSD ", 4) == 0)
			{
				//DSD
				dsdsdckDataSize = GET_DFF_INT64(dsdsdckData + 4);
				totalcount += dsdsdckDataSize + 12;
				psong->song_length = (int)((double)dsdsdckDataSize / (double)samplerate / (double)channels * 8 * 1000);

				//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "songlength is %d\n", psong->song_length);

				fseeko(fp, dsdsdckDataSize, SEEK_CUR);
			}
			else if (strncmp((char*)dsdsdckData, "DST ", 4) == 0)
			{
				//DST
				dsdsdckDataSize = GET_DFF_INT64(dsdsdckData + 4);
				totalcount += dsdsdckDataSize + 12;

				//DST Frame Information chunk
				if (!(rt = fread(frteckData, 18, 1, fp)))
				{
					DPRINTF(E_WARN, L_SCANNER, "Could not read DST Frame Information chunk from %s\n", file);
					fclose(fp);
					return -1;
				}

				if (strncmp((char*)frteckData, "FRTE", 4) == 0)
				{
					//uint64_t frteckDataSize = GET_DFF_INT64(frteckData+4);
					numFrames = GET_DFF_INT32((char*)frteckData + 12);
					frameRate = GET_DFF_INT16((char*)frteckData + 16);

					psong->song_length = numFrames / frameRate * 1000;

					fseeko(fp, dsdsdckDataSize - 18, SEEK_CUR);
				}
				else
				{
					DPRINTF(E_WARN, L_SCANNER, "Invalid DST Frame Information chunk in %s\n", file);
					fclose(fp);
					return -1;
				}

				//DST Sound Index Chunk
				if (!(rt = fread(dstickData, 12, 1, fp)))
				{
					if (ferror(fp))
					{
						DPRINTF(E_WARN, L_SCANNER, "Could not read DST Sound Index chunk from %s\n", file);
						fclose(fp);
						return -1;
					}
					else
					{
						//EOF
						break;
					}
				}

				if (strncmp((char*)dstickData, "DSTI", 4) == 0)
				{
					dstickDataSize = GET_DFF_INT64(dstickData + 4);
					totalcount += dstickDataSize + 12;
					fseeko(fp, dstickDataSize, SEEK_CUR);
				}
				else
				{
					fseeko(fp, -12, SEEK_CUR);
				}
			}
			else
			{
				DPRINTF(E_WARN, L_SCANNER, "Invalid DSD/DST Sound Data chunk in %s\n", file);
				fclose(fp);
				return -1;
			}
		}
		else if (!strncmp((char*)ckbuf, "COMT", 4))
		{
			//COMT Chunk
			comtckDataSize = GET_DFF_INT64(ckbuf + 4);
			totalcount += comtckDataSize + 12;
			fseeko(fp, comtckDataSize, SEEK_CUR);
		}
		else if (!strncmp((char*)ckbuf, "DIIN", 4))
		{
			//Edited Master Information chunk
			diinckDataSize = GET_DFF_INT64(ckbuf + 4);
			unsigned char diinckData[diinckDataSize];
			totalcount += diinckDataSize + 12;

			if (!(rt = fread(diinckData, diinckDataSize, 1, fp)))
			{
				DPRINTF(E_WARN, L_SCANNER, "Could not read Edited Master Information chunk from %s\n", file);
				fclose(fp);
				return -1;
			}

			uint64_t icount = 0;
			while (icount < diinckDataSize)
			{
				if (!strncmp((char*)diinckData + icount, "EMID", 4))
				{
					//Edited Master ID chunk
					icount += 4;
					icount += GET_DFF_INT64(diinckData + icount) + 8;
				}
				else if (!strncmp((char*)diinckData + icount, "MARK", 4))
				{
					//Master Chunk
					icount += 4;
					icount += GET_DFF_INT64(diinckData + icount) + 8;
				}
				else if (!strncmp((char*)diinckData + icount, "DIAR", 4))
				{
					//Artist Chunk
					icount += 4;
					diarckDataSize = GET_DFF_INT64(diinckData + icount);
					unsigned char arttext[diarckDataSize + 1 - 4];

					icount += 12;

					memset(arttext, 0x00, sizeof(arttext));
					strncpy((char*)arttext, (char*)diinckData + icount, sizeof(arttext) - 1);
					psong->contributor[ROLE_ARTIST] = strdup((char*)&arttext[0]);

					icount += diarckDataSize - 4;
				}
				else if (!strncmp((char*)diinckData + icount, "DITI", 4))
				{
					//Title Chunk
					icount += 4;
					ditickDataSize = GET_DFF_INT64(diinckData + icount);
					unsigned char titletext[ditickDataSize + 1 - 4];

					icount += 12;

					memset(titletext, 0x00, sizeof(titletext));
					strncpy((char*)titletext, (char*)diinckData + icount, sizeof(titletext) - 1);
					psong->title = strdup((char*)&titletext[0]);
					icount += ditickDataSize - 4;
				}
				else
				{
					break;
				}
			}
		}
		else if (!strncmp((char*)ckbuf, "MANF", 4))
		{
			//Manufacturer Specific Chunk
			manfckDataSize = GET_DFF_INT64(ckbuf + 4);
			totalcount += manfckDataSize + 12;
			fseeko(fp, manfckDataSize, SEEK_CUR);
		}
	}

	fclose(fp);

	//DPRINTF(E_DEBUG, L_SCANNER, "totalsize is 0x%016lx\n", (long unsigned int)totalsize);
	//DPRINTF(E_DEBUG, L_SCANNER, "propckDataSize is 0x%016lx\n", (long unsigned int)propckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "cmprckDataSize is 0x%016lx\n", (long unsigned int)cmprckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "abssckDataSize is 0x%016lx\n", (long unsigned int)abssckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "lscockDataSize is 0x%016lx\n", (long unsigned int)lscockDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "dsdsdckDataSize is 0x%016lx\n", (long unsigned int)dsdsdckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "dstickDataSize is 0x%016lx\n", (long unsigned int)dstickDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "comtckDataSize is 0x%016lx\n", (long unsigned int)comtckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "diinckDataSize is 0x%016lx\n", (long unsigned int)diinckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "diarckDataSize is 0x%016lx\n", (long unsigned int)diarckDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "ditickDataSize is 0x%016lx\n", (long unsigned int)ditickDataSize);
	//DPRINTF(E_DEBUG, L_SCANNER, "manfckDataSize is 0x%016lx\n", (long unsigned int)manfckDataSize);


	//DPRINTF(E_DEBUG, L_SCANNER, "Got dff fileinfo successfully=%s\n", file);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "TITLE is %s\n",psong->title );
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "ARTIST is %s\n",psong->contributor[ROLE_ARTIST]);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "samplerate is  %d\n", psong->samplerate);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "song_length is  %d\n", psong->song_length);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "channels are  %d\n", psong->channels);
	//DEBUG DPRINTF(E_DEBUG, L_SCANNER, "bitrate is  %d\n", psong->bitrate);

	xasprintf(&(psong->dlna_pn), "DFF");
	return 0;
}
