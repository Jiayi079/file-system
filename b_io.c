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
#include "fsDir.c"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer -- LBA used
	int buflen;		//holds how many valid bytes are in the buffer
	int b_flags; 	//holds the read/write file permissions
	int fs_FD; 		//holds the system's file descriptor
	int b_offset; 	//holds the position of where we are in the file

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
		
		//Using the file system's file descriptor, check if file is 
		//open, if not, print error message 
		if(fcbArray[fd].fs_FD == -1){

			printf("The file has not been opened yet/n");
			return -1;
		}

		//Checks the file permissions, if the file is not set to read
		//and write only, print error message 
		if(!fcbArray[fd].b_flags & O_WRONLY || !fcbArray[fd].b_flags & O_RDWR){

			printf("The file cannot be accessed due to R&W permissions\n");
			return -1;

		}

	//Change where we are in the file by incrementing the offset
	fcbArray[fd].b_offset += offset;
		
	return (fcbArray[fd].index); //Return the current position in the buffer
}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if(strlen(buffer) == 0){
  		exit(0);
 	}
 
 	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
	return (-1);      //invalid file descriptor
	}
  
	// file can not open for this descriptor
	if(fcbArray[fd].fs_FD == -1){
		exit(-1);
	}
 
	// file can not open for this descriptor
	if(!fcbArray[fd].b_flags & O_WRONLY || !fcbArray[fd].b_flags & O_RDWR){
		exit(-1);
	}

	if(count < 200){
	strncpy(fcbArray[fd].buf + fcbArray[fd].b_offset, buffer, count);

	fcbArray[fd].buflen += count;

	// the default file block count we set is as 10
	if(strlen(fcbArray[fd].buf) > 10*B_CHUNK_SIZE){

		//relase the space
		for (int i = directories[fcbArray[fd].index].directoryStartLocation;
            i < JCJC_VCB->numberOfBlocks; i++)
        {
            setBitFree(i, freespace);
        }

		// after free the space, we allocate a new one
		//to save not have over block
		int safe_size = (fcbArray[fd].buflen + (512 -1)) / B_CHUNK_SIZE;
		allocateFreeSpace_Bitmap(safe_size);

	}

	//we set the buffer maximum size is 50
	LBAwrite(fcbArray[fd].buf, 50, fcbArray[fd].index);

	//then return the value of number we have written
	return (strlen(buffer));
	}
 
 
	//copy the length of the the buffer into our buf
	strncpy(fcbArray[fd].buf + fcbArray[fd].b_offset, buffer, count);
	fcbArray[fd].buflen += strlen(buffer);
	fcbArray[fd].b_offset += strlen(buffer);

	//then return the value of number we have written
	return (strlen(buffer));
}

// return the exactly directory we find
// return NULL if not found anything
fdDir * fs_opendir(const char *name)
{
    // copy the name to avoid modifying it
    char *path = malloc(strlen(name) + 1);
    if (path == NULL)
    {
        printf("[mfs.c -- fs_opendir] malloc path failed\n");
        return NULL;
    }

    strcpy(path, name);
    directories = getDirByPath(path);

    // set the entry index to 0 for fs_readDir() works
    openedDirEntryIndex = 0;

    free(path);
    path = NULL;
    return directories;
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
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	return (0);	//Change this
	}
	
//Interface to Close the file	
int b_close (b_io_fd fd)
	{
	fcbArray[fd].fs_FD = -1; //close the file handle
	free(fcbArray[fd].buf);	   //free the associated buffer
	fcbArray[fd].buf = NULL;   //set it be null
	fcbArray[fd].index = -1;
	fcbArray[fd].b_flags = -1;
	fcbArray[fd].buflen = 0; //holds how many valid bytes are in the buffer
	fcbArray[fd].b_offset = 0; //holds the position of where we are in the file
	}
