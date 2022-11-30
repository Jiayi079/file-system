/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
// #include "mfs.c"
#include "helperFunctions.h"
#include "helperFunctions.c"

#define Magic_Number 0x434A434A //"JCJC" converted to Hex (Big Endian)

// int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB);

// int dir_DE_count = 50; //set 50 for now as an example

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	
	//Using the size of our VCB and mod with blockSize, determine how
	//many blocks are needed when initializing file system
	uint64_t blockCount_VCB = sizeof(JCJC_VCB) / blockSize;

	if(sizeof(JCJC_VCB) % blockSize > 0){
		blockCount_VCB++;
	}

	// printf("[debug]blockCount_VCB: %d\n",blockCount_VCB);

	//Malloc a block of memory for our VCB pointer and set LBAread block 0
	//Basically, init a VCB buffer and read/start from block 0 of VCB
	char *vcb_Buffer = malloc(blockCount_VCB * blockSize);
	if(vcb_Buffer == NULL){

		printf("Failed to allocate the buffer for VCB\n");
		return -1;
	}

	// printf("[debug]size of vcb_Buffer: %s\n", vcb_Buffer);

	// LBAread(vcb_Buffer, blockCount_VCB, 0);
	LBAread(vcb_Buffer, 1, 0);

	//Malloc a block of space for the VCB so that 
	//we can copy whatever is in the buffer into our VCB
	JCJC_VCB = malloc(sizeof(volume_ControlBlock));
	// JCJC_VCB = malloc(blockSize);
	// LBAread(JCJC_VCB, 1, 0); // VCB starting block -> 0
	if(JCJC_VCB == NULL){

		printf("Failed to allocate space in VCB\n");
		return -1;
	}

	memcpy(JCJC_VCB, vcb_Buffer, sizeof(volume_ControlBlock));

	//Free our VCB buffer and set equal to NULL
	free(vcb_Buffer);
	vcb_Buffer = NULL;

	//Check if volume is formated & initialized by cross-matching
	//the Magic Number we previously defined. If not, then init
	if (Magic_Number == JCJC_VCB -> magicNumber){

		//Malloc our VCB buffer to read the amount of freespace 
		//from the volume to the VCB buffer
		vcb_Buffer = malloc(JCJC_VCB -> freeSpace_BlockCount * JCJC_VCB -> blockSize);
		if (vcb_Buffer == NULL){

			printf("failed to malloc vcb_Buffer\n");
			return -1;
		}

		LBAread(vcb_Buffer, JCJC_VCB -> freeSpace_BlockCount, JCJC_VCB -> VCB_blockCount);

		// copy the needed amount of block space into freespace
		freespace = malloc(JCJC_VCB->numberOfBlocks);
		if (freespace == NULL)
		{
			printf("failed to malloc freespace\n");
			return -1;
		}

		memcpy(freespace, vcb_Buffer, JCJC_VCB->numberOfBlocks);

		//Free our VCB buffer and set equal to NULL
		free(vcb_Buffer);
		vcb_Buffer = NULL;

		//Set the root directory as our initial working directory
		vcb_Buffer = malloc(getVCB_BlockCount(sizeof(fdDir)) * JCJC_VCB -> blockSize);
		if (vcb_Buffer == NULL)
		{
			printf("failed to malloc vcb_Buffer for root directory\n");
			return -1;
		}

		LBAread(vcb_Buffer, getVCB_BlockCount(sizeof(fdDir)), JCJC_VCB->location_RootDirectory);

		// Initialize our root directory pointer for our VCB
		fs_CWD = malloc(sizeof(fdDir));
		if (fs_CWD == NULL)
		{
			printf("failed to malloc root directory\n");
			return -1;
		}

		memcpy(fs_CWD, vcb_Buffer, sizeof(fdDir));

		//Free our VCB buffer and set equal to NULL
		free(vcb_Buffer);
		vcb_Buffer = NULL;


	} else {

		//If all values are not at defualt 0, then volume is not formatted 
		if (init_VCB(numberOfBlocks, blockSize, blockCount_VCB) != 0)
		{
			printf("VCB has not been formated and initialized\n");
			return -1;
		}
		// LBAwrite(vcb_Buffer, 1, 0);

		// printf("*******test before init_freeSpace()\n");
		if (init_freeSpace() != 0)
		{
			printf("Freespace has not been formated and initialized\n");
			return -1;
		}

		// printf("*******test before RootDir()\n");

		if (init__RootDir() != 0)
		{
			printf("Root Directory has not been formated and initialized\n");
			return -1;
		}

		// write vcb into the disk
		LBAwrtie_func(JCJC_VCB, sizeof(volume_ControlBlock), 0);
	}


	//VCB status debugging 	
	// LBAread
	// LBAwrite(, 5, 0);
	printf("*** VCB STATUS ***\n");
	printf("number of blocks: %ld\n", JCJC_VCB->numberOfBlocks);
	printf("block size: %ld\n", JCJC_VCB->blockSize);
	printf("vcb block count: %d\n", JCJC_VCB->VCB_blockCount);
	printf("freespace block count: %d\n", JCJC_VCB->freeSpace_BlockCount);
	printf("first free block index: %ld\n\n", JCJC_VCB->current_FreeBlockIndex);

	// printf("dir_DE_count: %d\n", dir_DE_count);

	// setting the other status in memory
	rootDir = NULL;
	openedDirEntryIndex = 0;

	return 0;
}


void exitFileSystem ()
{
	printf ("System exiting\n");
}

//Initializing the VCB
int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB)
{
	// Using memset(), initialize a block of memory for our VCB, with value 0
	// based on the size of our VCB
	memset(JCJC_VCB, 0, sizeof(volume_ControlBlock));

	// Initialize our VCB with these deault values
	JCJC_VCB->magicNumber = Magic_Number;
	JCJC_VCB->numberOfBlocks = numberOfBlocks;
	JCJC_VCB->blockSize = blockSize;
	JCJC_VCB->VCB_blockCount = blockCount_VCB; // Amount of blocks used by the VCB
	JCJC_VCB->current_FreeBlockIndex = 0;
	// *** TODO: need to add rootLocation, free_block_count, location_RootDirectory


	// Since 1 byte consists of 8 bits, we need to find
	// the number of bytes used for each block in the VCB
	// then we can get the number of blocks needed for the initialized VCB
	uint64_t bytes_PerBlock = numberOfBlocks / 8; //2441 + 1 = 2442
	if (numberOfBlocks % 8 > 0)
	{
		bytes_PerBlock++;
	}

	JCJC_VCB->freeSpace_BlockCount = getVCB_BlockCount(bytes_PerBlock);
	// printf("freespace_blockcount: %ld\n", JCJC_VCB->freeSpace_BlockCount);

	return 0;

}

// init freespace
int init_freeSpace()
{
	uint64_t bits_in_block = 8 * JCJC_VCB->blockSize;
	uint64_t block_count = JCJC_VCB->numberOfBlocks / bits_in_block;
	if (JCJC_VCB->numberOfBlocks % bits_in_block != 0)
	{
		block_count++;
	}

	// printf("block_count: %d\n", block_count);

	JCJC_VCB->freeSpace_BlockCount = block_count;

	//init the bitmap array -> 5 blocks
	freespace = malloc(block_count * JCJC_VCB->blockSize);

	// [ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ]
	// [VCB][FRE][FRE][FRE][FRE][FRE][   ]
	if (freespace != NULL)
	{
		memset(freespace, 0, JCJC_VCB->numberOfBlocks);
	}
	else{
		printf("freespace malloc failed\n");
		return -1;
	}

	// printf("[debug]current_freeblockIndex: %ld\n", JCJC_VCB->current_FreeBlockIndex);

	// free(freespace);
	// freespace = NULL;

	// Set current block as" used" in the free space
	// and update it to the VCB

	// set the bitmap and the currently-used block that are being used by VCB.
	int i = allocateFreeSpace_Bitmap(JCJC_VCB->freeSpace_BlockCount + JCJC_VCB->VCB_blockCount);

	// printf("debug 2\n");
	// printf("allocated free space: %d\n", i);
	return i;
}


int init__RootDir()
{
	// enter NULL to serve as the root directory
	fdDir *rootDir;
	rootDir = malloc(sizeof(fdDir));

	if (rootDir == NULL)
	{
		printf("[fsInit.c -- init__RootDir] malloc rootDir failed\n");
		return -1;
	}

	// write the directory file psycially
	LBAwrtie_func(rootDir, rootDir->d_reclen, rootDir->directoryStartLocation);

    // read the data again if it is updating cwd
    if (fs_CWD != NULL && rootDir->directoryStartLocation == fs_CWD->directoryStartLocation)
    {
        memcpy(fs_CWD, rootDir, sizeof(fdDir));
    }

	// set the root directory as cwd
	fs_CWD = rootDir;
	JCJC_VCB->location_RootDirectory = fs_CWD->directoryStartLocation;
	return 0;
}