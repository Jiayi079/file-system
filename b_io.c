/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
{
	int fs_FD;		 // holds the systems file descriptor
	char *buf;		 // holds the open file buffer
	uint64_t index;	 // holds the current position in the buffer
	uint64_t buflen; // holds how many valid bytes are in the buffer
	fdDir *parent;	 // holds the parent directory of the file
	char *fileName;	 // holds the true file name not the path
	int b_flags;	 // holds the functionality of the method
} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
{
	b_io_fd returnFd;

	int fd;
	int index_LBA;

	if (startup == 0) 
	{
		b_init();  //Initialize our system
	}

	char path[256];

	if (strlen(cwd) > 1) // check if cwd already is a path
	{
		strcpy(path, cwd);
        strcat(path, "/"); // copy cwd to path by adding a slash
		strcat(path, filename); // add filename after adding slash
	}
	else
    {
		// if cwd is not a path, copy cwd to path without adding a slash
        strcpy(path, cwd);
        strcat(path, filename);
    }

	// create a temporary directory entry

	char * buffer;

	// TODO: not sure the maximum block count here, set 50 first?
	buffer = (char *)malloc(50 * 512);
	// TODO: not sure the buffer maximum count size
	int bufferMax = 50;

	Directory_Entry * de;
	int tempIndex = -1; // set to -1 for now to check error later
	for (int i = 0; i < 20; i++)
	{
		de = (Directory_Entry *)directories[i].dirEntry[0];
		if (strcmp(de->filePath, path) == 0)
		{
			// if finding the path in de successfully
			// keep the index i for later using
			tempIndex = i;
            break;
		}
	}

	if (tempIndex != -1) // means we found the file in the directory
	{
		if (directories[tempIndex].fileType == 0) // 0 -> dir, 1 -> file
		{
			return -1; // terminate open function
		}
		else // otherwise this file type is a file
		{
			char * readedBuffer;
			int offset = 0;
			int alreadyReadedCount = 0;
			readedBuffer = (char *)malloc(512);
			// buffer, 50(max buffer count), directories[tempIndex].directoryStartLocation
			for (int i = directories[tempIndex].directoryStartLocation; 
				i < JCJC_VCB->numberOfBlocks; i++)
			{
				if (bufferMax == 0)
				{
					break;
				}
				LBAread(readedBuffer, 1, i);
				// TODO: need to check signal??
				offset = alreadyReadedCount * 512;
				alreadyReadedCount++;
				strcpy(buffer + offset, readedBuffer);
				bufferMax--;
			}
			// free readedBuffer which is only used in the for loop
			free(readedBuffer);
            readedBuffer = NULL;
		}
	}


	if (tempIndex == -1) // means the file does not exist
    {
		if ((flags & O_CREAT) == 0)
		{
			return -1; // terminate open function
		}

		int index_LBA = allocateFreeSpace_Bitmap(10);

		Directory_Entry * child_de;
		setDirectoryEntry(child_de, filename, 10 * JCJC_VCB->blockSize, 1, path, 1);

		int cwdIndex;
		// get cwdIndex
		for (int i = 0; i < 20; i++)
		{
			child_de = (Directory_Entry *)directories[i].dirEntry[0];
			if (strcmp(child_de->filePath, path) == 0)
			{
				cwdIndex = i;
				break;
			}
		}

		// TODO: may add check if cwdIndex is -1 ?

		int check = -1;
		for (int i = 0; i < 8; i++)
        {
			de = (Directory_Entry *)directories[cwdIndex].dirEntry[i];
			if (de->dirUsed == 0)
			{
				check = i;
				// don't have to used break, we need to keep the last one
			}
		}

		// check if check is -1, which means child directory set failed
		if (check == -1)
		{
			printf("[b_io.c -- b_open()]: child directory set failed\n");
		}

		memcpy(directories[cwdIndex].dirEntry[check], (char *)child_de, 512);

		// find unused location in directory
		int unused = -1;
		for (int i = 1; i < 20; i++)
		{
			if (directories[i].isUsed == 0)
			{
				unused = i;
                break;
			}
		}

		if (unused!= -1)
        {
			printf("[b_io.c -- b_open()]: Don't have enough space in directory\n");
			return -1;
		}

		strcpy(directories[unused].d_name, filename);
		directories[unused].isUsed = 1;
		directories[unused].fileType = 1;
		directories[unused].dirLBA = index_LBA;

		// TODO: need to add those value??
		// unsigned short  d_reclen;
		// uint64_t blockIndex;
		// struct fs_diriteminfo dir_DE_count[MAX_DE];
		// unsigned int current_location;

		Directory_Entry * curr_de;
		setDirectoryEntry(curr_de, ".", sizeof(fdDir), 0, path, 1);

		Directory_Entry * par_de;
		setDirectoryEntry(par_de, "..", sizeof(fdDir), 0, cwd, 1);

		memcpy(directories[unused].dirEntry[0], (char *)curr_de, 512);
		memcpy(directories[unused].dirEntry[1], (char *)par_de, 512);

		LBAwrite((char *)directories, length_of_dir, JCJC_VCB->location_RootDirectory);

		free(curr_de);
		curr_de = NULL;
		free(par_de);
		par_de = NULL;
        free(child_de);
		child_de = NULL;
	}



	//*** TODO ***:  Modify to save or set any information needed
	//
	//
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	
	if (returnFd == -1)
    {
		printf("[b_io.c -- b_open()]: b_getFCB() failed\n");
		return -1;
	}

	fcbArray[returnFd].fs_FD = fd; // Save the linux file descriptor
	fcbArray[returnFd].index = index_LBA;
	fcbArray[returnFd].b_flags = flags;

	if (buffer != NULL)
	{
		fcbArray[returnFd].buf = calloc(20 * B_CHUNK_SIZE, sizeof(char));
		strcpy(fcbArray[returnFd].buf, buffer);
		fcbArray[returnFd].buflen = 0; // have not read anything yet
		fcbArray[returnFd].b_offset = 0; // have not read anything yet
		return (returnFd);			   // all set
	}
	else
    {
		fcbArray[returnFd].buf = malloc(20 * B_CHUNK_SIZE);
		fcbArray[returnFd].buflen = 0; // have not read anything yet
		fcbArray[returnFd].b_offset = 0; // have not read anything yet
    }

	if (fcbArray[returnFd].buf == NULL)
	{
		b_close(returnFd);
		return -1;
	}

	return (returnFd);						// all set
}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	//If offset position is at the beginning, then
	//set position of offset to the beginning of file
		if (whence & SEEK_SET)
	{

		fcbArray[fd].index = offset;

	}

	//If offset position is at the current position, then
	//set position of offset to the curent pointer of the file
	if (whence & SEEK_CUR)
	{

		fcbArray[fd].index += offset;

	}

	//If offset position is at the end, then
	//this means we are the end of file
	if (whence & SEEK_END)
	{
	
		fcbArray[fd].index = fcbArray[fd].file -> fileSize / B_CHUNK_SIZE;

	}

	//Return the current file position in our buffer 
	return (fcbArray[fd].index); 
}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	
	if (startup == 0)
		b_init(); // Initialize our system

	if ((fd < 0) || (fd >= MAXFCBS) || fcbArray[fd].fs_FD == -1 || count < 0)
	{
		return (-1);
	}

	// initialize the detector the first time it calls this function
	if (fcbArray[fd].b_flags == 0)
	{
		fcbArray[fd].b_flags = 2; // set become 2 means write, and 1 means read

		// check if there is no more place to store files in parent directory
		if (fcbArray[fd].parent->dirEntryPosition >= MAX_ENTRIES_NUMBER)
		{
			fcbArray[fd].fs_FD = -2;
			return -1;
		}

		// check if there is already a same name of file
		for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
		{
			if (fcbArray[fd].parent->dirEntry[i].dirUsed == SPACE_IN_USED &&
				strcmp(fcbArray[fd].parent->dirEntry[i].file_name, fcbArray[fd].fileName) == 0)
			{
				printf("\nsame name of directory or file existed\n");
				fcbArray[fd].fs_FD = -2;
				return -1;
			}
		}

		// allocate the buffer with the first size
		fcbArray[fd].buf = malloc(B_CHUNK_SIZE);
		if (fcbArray[fd].buf == NULL)
		{
			printf("malloc() on fcbArray[returnFd].buf");
			fcbArray[fd].fs_FD = -2;
			return -1;
		}
		fcbArray[fd].buflen += B_CHUNK_SIZE;
	}

	// it shouldn't do another functionality
	if (fcbArray[fd].b_flags != 2) // check if the b_flags is present to write
	{
		// printf("no mix use of functionality!");
		return -1;
	}

	// if it just reaches EOF, we skip the copy process
	// this is a rare case for file with mutiples of the buffer sizes
	if (count != 0)
	{
		// calculate the index to determine realloc()
		uint64_t newIndex = fcbArray[fd].index + count;
		if (newIndex > fcbArray[fd].buflen)
		{
			// realloc() with the new length
			fcbArray[fd].buflen += B_CHUNK_SIZE;
			fcbArray[fd].buf = realloc(fcbArray[fd].buf, fcbArray[fd].buflen);
			if (fcbArray[fd].buf == NULL)
			{
				printf("realloc() on fcbArray[argfd].buf");
				return -1;
			}
		}

		// copy into our buffer and set the new index
		memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer, count);
		fcbArray[fd].index = newIndex;
	}

	return 0;

}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char * buffer, int count)
{
	if (startup == 0)
		b_init(); // Initialize our system

	if ((fd < 0) || (fd >= MAXFCBS) || fcbArray[fd].fs_FD == -1 || count < 0)
	{
		printf("[b_io -- b_read] conidtion error\n");
		return (-1); 					//invalid file descriptor
	}

	// check that fd is between 0 and (MAXFCBS-1)
	// also check if the fd value in the fcbArray is not -1 and the count is not less than 0
	if (fcbArray[fd].b_flags == 0)
	{
		fcbArray[fd].b_flags = 1; // set become 1 means read, and 2 means write
		int indexHolder = -1;
		for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
		{
			// check if it's parent directory is used
			// check if the parent directory's file type is file nor dir
			// check if the parent name is the same as the file name we have in the fileName
			// condition should be all checked to make sure we are on the right track
			if (fcbArray[fd].parent->dirEntry[i].dirUsed == SPACE_IN_USED &&
				fcbArray[fd].parent->dirEntry[i].fileType == FILE_TYPE &&
				strcmp(fcbArray[fd].parent->dirEntry[i].file_name, fcbArray[fd].fileName) == 0)
			{
				indexHolder = i; // keep i's value used later to check
				// set up the buffer length become the exactly dirEntry's file size
				fcbArray[fd].buflen = fcbArray[fd].parent->dirEntry[i].fileSize;
				// get the total block count we need
				// and we have to set up the b_io buffer size, and vcb same avoid the error
				int blockCount = getVCB_BlockCount(fcbArray[fd].buflen);
				fcbArray[fd].buf = malloc(blockCount * B_CHUNK_SIZE);

				// check if the buff malloc is successful
				if (fcbArray[fd].buf == NULL)
				{
					printf("[b_io.c -- b_read] fcbArray[fd].buf malloc failed\n");
					fcbArray[fd].fs_FD = -2;
					return -1;
				}

				// read data into our fcbArray[fd].buf by using LBAread
				LBAread(fcbArray[fd].buf, blockCount, fcbArray[fd].parent->dirEntry[i].dir_Location);

				// after we finish the read data, we simply break out of the loop
				break;
			}
		}

		// handle error of not find files
		if (indexHolder == MAX_ENTRIES_NUMBER)
		{
			printf("[b_io.c -- b_read] indexHolder is out of range\n");
			return -1;
		}
	}

	// check if the b_flags is set to become 1 -> READ
	if (fcbArray[fd].b_flags != 1)
	{
		printf("[b_io.c -- b_read] fcbArray[fd].b_flags is not set up to READ -> 1\n");
		return -1;
	}

	int readed = 0; // hold the number of bytes readed into the buffer
	uint64_t remain = fcbArray[fd].buflen - fcbArray[fd].index;

	// check remain is bigger than 0
	// which means we still need read data
	if (remain > 0)
	{
		if (remain < count) // if true, means this is the last time of read
		{
			readed = remain; // store the number of bytes remainning into the readed
		}
		else // if false, means we have to keep offset and run read function again
		{
			readed = count;
		}

		// copy the number of readed data into the buffer at this time
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, readed);

		// update the index location since we may run b_read function again
		fcbArray[fd].index += readed;
	}

	return readed; // return the number of bytes readed
}
	
// Interface to Close the file	
// should close by using file descriptor and set the space become free
int b_close(b_io_fd fd)
{
	// check for some error that return a invalid fd
	// must handle memory leak above (no time to optimize better)
	if (fcbArray[fd].fs_FD != -1 && fcbArray[fd].fs_FD != -2)
	{
		// free all associated malloc() pointer
		if (fcbArray[fd].buf != NULL)
		{
			free(fcbArray[fd].buf);
			fcbArray[fd].buf = NULL;
		}
		if (fcbArray[fd].parent != NULL)
		{
			free(fcbArray[fd].parent);
			fcbArray[fd].parent = NULL;
		}
		if (fcbArray[fd].fileName != NULL)
		{
			free(fcbArray[fd].fileName);
			fcbArray[fd].fileName = NULL;
		}
	}
	fcbArray[fd].fs_FD = -1;
	return 0;
}