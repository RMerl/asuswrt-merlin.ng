/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:license-private
 *
 *  Application running in Linux host to convert software image from
 *  binary to hexadecimal ascii
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define END_OF_LINE "\n"
#define DATA_BUFFER_SIZE    2000   // 2000 bytes for data

int bin2hex(char *inFile, char *outFile)
{
    int i = 0, j = 0;
    int countRd = 0, countWr = 0;
    int exitCode = -1;
    int fdRead = -1, fdWrite = -1;
    unsigned char binaryBuf[DATA_BUFFER_SIZE], hexStr[(DATA_BUFFER_SIZE*2)+1];

    if ((fdRead = open(inFile, O_RDONLY)) == -1)
    {
        printf("Failed to open '%s' for read: %s\n", inFile, strerror(errno));
        goto out;
    }

    if ((fdWrite = open(outFile,
                        O_CREAT | O_WRONLY,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) == -1)
    {
        printf("Failed to open '%s' for write: %s\n", outFile, strerror(errno));
        goto out;
    }

    memset(binaryBuf, 0, DATA_BUFFER_SIZE);
    memset(hexStr, 0, (DATA_BUFFER_SIZE*2) + 1);

    while ((countRd = read(fdRead, (void *)binaryBuf, DATA_BUFFER_SIZE)) > 0)
    {
        for (i = 0, j = 0; i < countRd; i++, j += 2)
        {
            sprintf(&(hexStr[j]), "%02x", binaryBuf[i]);
        }
        countWr = write(fdWrite, (void *)hexStr, countRd * 2);
        if (countWr != (countRd * 2))
        {
            printf("Number of written bytes %d is not double number of read bytes %d\n", countWr, countRd);
            goto out;
        }
        // mark end of line
        write(fdWrite, (void *)END_OF_LINE, strlen(END_OF_LINE));
    }

    // mark end of file
    write(fdWrite, (void *)END_OF_LINE, strlen(END_OF_LINE));

    exitCode = 0;

out:
    if (fdRead != -1)
        close(fdRead);

    if (fdWrite != -1)
        close(fdWrite);

    return exitCode;
}

void usage(char *progName)
{
    printf("usage: %s [-i inFile] [-o outFile]\n", progName);
    printf("       i: binary input file name\n");
    printf("       o: hex output file name\n");
}

int main(int argc, char *argv[])
{
    int  c = 0, exitCode=0;
    char *inFile = NULL, *outFile = NULL;

    /* parse command line args */
    while ((c = getopt(argc, argv, "i:o:")) != -1)
    {
        switch(c)
        {
            case 'i':
                inFile = optarg;
                break;
            case 'o':
                outFile = optarg;
                break;
            default:
                usage(argv[0]);
                exit(-1);
        }
    }

    if (inFile == NULL || outFile == NULL)
    {
        usage(argv[0]);
        exit(-1);
    }

    exitCode = bin2hex(inFile, outFile);

    return exitCode;
}
