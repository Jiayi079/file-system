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

#define Magic_Number 123

// int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB);


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


	//Check if volume is formated & initialized by cross-matching
	//the Magic Number we previously defined. If not, then init
	if (Magic_Number == JCJC_VCB -> magicNumber){


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
		JCJC_VCB -> VCB_blockCount = blockCount_VCB;
		JCJC_VCB -> current_FreeBlockIndex = 0;
		JCJC_VCB -> magicNumber = Magic_Number;



		return 0;

	}

