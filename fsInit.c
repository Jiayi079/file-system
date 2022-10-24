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

typedef struct VCB{

	int num_blocks;  			//integer variable for the number of blocks in VCB
	int blockSize = 512;			//variable bytes for the size of each blocks in VCB 
	int rootLocation [256];		//integer variable for root directory location 

	int freeSpace = 2560; 			//integer variable bytes for free block space left in VCB
	
	//integer variable for magic number, useful when opening files based on their 
	//File Signature, and also for hex dumps 
	long signature;				

	int * free_block_ptr;			//the pointer to track our free space
	int  free_block_count;			//the total numbers of the free blocks

}volume_ControlBlock;



int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}
