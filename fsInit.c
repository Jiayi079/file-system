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

#include "fsLow.h"
#include "mfs.h"

#define Magic_Number 123456


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	//Using the size of our VCB and mod with blockSize, determine how
	//many blocks are needed when initializing file system
	unsigned int blockCount_VCB = sizeof(JCJC_VCB) / blockSize;

	if(sizeof(JCJC_VCB) % blockSize > 0){
		blockCount_VCB++;
	}

	//Malloc a block of memory for our VCB pointer and set LBAread block 0
	//Basically, init a VCB buffer and read/start from block 0 of VCB
	char * vcb_Buffer = malloc(blockCount_VCB * blockSize);
	if(vcb_Buffer == NULL){

		printf("Failed to allocate the buffer for VCB");
		return -1;
	}

	LBAread(vcb_Buffer, blockCount_VCB, 0);

	//Malloc a block of space for the VCB so that 
	//we can copy whatever is in the buffer into our VCB
	JCJC_VCB = malloc(sizeof(volume_ControlBlock));
	if(JCJC_VCB == NULL){

		printf("Failed to allocate space in VCB");
		return -1;
	}

	memcpy(JCJC_VCB, vcb_Buffer, sizeof(volume_ControlBlock));

	//Free our VCB buffer and set equal to NULL
	free(vcb_Buffer);
	vcb_Buffer == NULL;

	//Check if volume is formated & initialized by cross-matching
	//the Magic Number we previously defined. If not, then init
	if (Magic_Number == JCJC_VCB -> magicNumber){

		//Malloc our VCB buffer to read the amount of freespace 
		//from the volume to the VCB buffer
		vcb_Buffer = malloc(JCJC_VCB -> freeSpace_BlockCount * JCJC_VCB -> blockSize);
		if(vcb_Buffer == NULL){

			printf("failed to malloc vcb_Buffer");
			return -1;
		}

		LBAread(vcb_Buffer, JCJC_VCB -> freeSpace_BlockCount, JCJC_VCB -> VCB_blockCount);


	} else {

		

	}


	//VCB status debugging 
	printf("*****VCB Status Overview*****");
	printf("VCB has this number of blocks: %ld", JCJC_VCB -> numberOfBlocks);
	printf("VCB has this block size: %ld", JCJC_VCB -> blockSize);

	
	return 0;
	}




	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}

//Initializing the VCB
int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB)
	{

		//Using memset(), initialize a block of memory for our VCB, with value 0
		//based on the size of our VCB
		memset(JCJC_VCB, 0, sizeof(volume_ControlBlock));

		//Initialize our VCB with these deault values 
		JCJC_VCB -> numberOfBlocks = numberOfBlocks;
		JCJC_VCB -> blockSize = blockSize;
		JCJC_VCB -> VCB_blockCount = blockCount_VCB; //Amount of blocks used by the VCB
		JCJC_VCB -> current_FreeBlockIndex = 0;
		JCJC_VCB -> magicNumber = Magic_Number;

		//Since 1 byte consists of 8 bits, we need to find
		//the number of bytes used for each block in the VCB
		//then we can get the number of blocks needed for the initialized VCB


		// u_int64_t bytes_PerBlock = numberOfBlocks / 8;
		// if(numberOfBlocks % 8 > 0){

		// 	bytes_PerBlock++;

		// }

		// JCJC_VCB -> freeSpace_BlockCount = getVCB_BlockCount(bytes_PerBlock);


		return 0;

	}