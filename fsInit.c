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
#include "helperFunctions.c"
#include "helperFunctions.h"
#include "bitmap.c"

#define Magic_Number 123456 //Will change later, temp placeholder

// #define Magic_Number 123
// int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB);

// int dir_DE_count = 50; //set 50 for now as an example



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

	// printf("[debug]blockCount_VCB: %d\n",blockCount_VCB);

	//Malloc a block of memory for our VCB pointer and set LBAread block 0
	//Basically, init a VCB buffer and read/start from block 0 of VCB
	char * vcb_Buffer = malloc(blockCount_VCB * blockSize);
	if(vcb_Buffer == NULL){

		printf("Failed to allocate the buffer for VCB\n");
		return -1;
	}

	// printf("[debug]size of vcb_Buffer: %s\n", vcb_Buffer);

	LBAread(vcb_Buffer, blockCount_VCB, 0);

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
	vcb_Buffer == NULL;

	//Check if volume is formated & initialized by cross-matching
	//the Magic Number we previously defined. If not, then init
	if (Magic_Number == JCJC_VCB -> magicNumber){

		//Malloc our VCB buffer to read the amount of freespace 
		//from the volume to the VCB buffer
		vcb_Buffer = malloc(JCJC_VCB -> freeSpace_BlockCount * JCJC_VCB -> blockSize);
		if(vcb_Buffer == NULL){

			printf("failed to malloc vcb_Buffer\n");
			return -1;
		}

		LBAread(vcb_Buffer, JCJC_VCB -> freeSpace_BlockCount, JCJC_VCB -> VCB_blockCount);

		// copy the needed amount of block space into freespace
		freespace = malloc(JCJC_VCB -> numberOfBlocks);
		if (freespace == NULL)
		{
			printf("failed to malloc freespace\n");
			return -1;
		}

		memcpy(freespace, vcb_Buffer, JCJC_VCB -> numberOfBlocks);

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

		LBAread(vcb_Buffer, getVCB_BlockCount(sizeof(fdDir)), JCJC_VCB -> location_RootDirectory);

		//Initialize our root directory pointer for our VCB
		rootDir_ptr = malloc(sizeof(fdDir));
		if (rootDir_ptr == NULL)
		{
			printf(" failed to malloc root directory\n");
			return -1;
		}

		memcpy(rootDir_ptr, vcb_Buffer, sizeof(fdDir));

		//Free our VCB buffer and set equal to NULL
		free(vcb_Buffer);
		vcb_Buffer = NULL;

		// ----------------------------
		// free(rootDir_ptr);
		// rootDir_ptr = NULL;


	} else {

		//If all values are not at defualt 0, then volume is not formatted 
		if (init_VCB(numberOfBlocks, blockSize, blockCount_VCB) != 0)
		{

			printf("VCB has not been formated and initialized\n");
			return -1;
		}
		LBAwrite(vcb_Buffer, 1, 0);

		// init_freeSpace(JCJC_VCB);
		if (init_freeSpace(JCJC_VCB) != 6)
		{
			printf("Freespace has not been formated and initialized\n");
			return -1;
		}

		// if (init__RootDir() != 0)
		// {
		// 	printf("Root Directory has not been formated and initialized\n");
		// 	return -1;
		// }
		init__RootDir();
	}


	//VCB status debugging 	
	// LBAread
	// LBAwrite(, 5, 0);
	printf("***********VCB Status Overview***********\n");
	printf("VCB has this number of blocks: %ld\n", JCJC_VCB -> numberOfBlocks);
	printf("VCB has this block size: %ld\n", JCJC_VCB -> blockSize);
	printf("VCB has this block count: %d\n", JCJC_VCB -> VCB_blockCount);
	printf("Free Space has this many blocks: %ld\n", JCJC_VCB -> freeSpace_BlockCount);
	printf("*****************************************\n");

	// printf("dir_DE_count: %d\n", dir_DE_count);

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
	JCJC_VCB -> first_freespace = 1;
	// *** TODO: need to add rootLocation, free_block_count, location_RootDirectory


	//Since 1 byte consists of 8 bits, we need to find
	//the number of bytes used for each block in the VCB
	//then we can get the number of blocks needed for the initialized VCB
	u_int64_t bytes_PerBlock = numberOfBlocks / 8;
	if(numberOfBlocks % 8 > 0){
		bytes_PerBlock++;
	}

	JCJC_VCB -> freeSpace_BlockCount = getVCB_BlockCount(bytes_PerBlock);
	// printf("freespace_blockcount: %ld\n", JCJC_VCB->freeSpace_BlockCount);


	return 0;

}


//init freespace
int init_freeSpace(volume_ControlBlock * JCJC_VCB){

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

	// JCJC_VCB->current_FreeBlockIndex = 0;

	if(freespace == NULL){
		printf("freespace malloc failed\n");
		exit (-1);
	}
	// -----------------------------------------------------------------------

	// printf("numberofblock: %d\n", JCJC_VCB->numberOfBlocks);
	// printf("freespace size: %d\n", sizeof(freespace));
	// check pointer is at least 2442 bytes
	// if (freespace)

	//0 -> free, 1 -> allocated
	//Set the free space array total as the numberOfBlocks(19531 bits)
	// memset(freespace, 0, JCJC_VCB -> numberOfBlocks);

	// 0 --> vcb  1-5 --> bitmap
	// memset(freespace, 1, JCJC_VCB->VCB_blockCount + JCJC_VCB -> freeSpace_BlockCount);
		// comment part
	// --------------------------------------------------------------------



	// [ 0 ][ 1 ][ 2 ][ 3 ][ 4 ][ 5 ][ 6 ]
	// [VCB][FRE][FRE][FRE][FRE][FRE][   ]
	// JCJC_VCB->numberOfBlocks / JCJC_VCB->blockSize;

	JCJC_VCB->current_FreeBlockIndex += JCJC_VCB->VCB_blockCount + JCJC_VCB -> freeSpace_BlockCount;

	// memset(freespace, 0, block_count * JCJC_VCB->blockSize * 8);
	// memset(freespace, 1, JCJC_VCB->first_freespace + block_count);

	for (int i = 0; i < JCJC_VCB->first_freespace + block_count; i++)
	{
		setBitUsed(i, freespace);
	}



	for (int i = JCJC_VCB->first_freespace + block_count; i < block_count * JCJC_VCB->blockSize; i++)
	{
		setBitFree(i, freespace);
	}

	// printf("[debug]initial freespace\n");

	//Write 5 blocks starting from block 1 
	int LBAwrite_return = LBAwrite(freespace, block_count, 1);


	if (LBAwrite_return != block_count)
	{
		printf("LBAwrite failed!\n");
	}

	// printf("[debug]current_freeblockIndex: %ld\n", JCJC_VCB->current_FreeBlockIndex);

	// free(freespace);
	// freespace = NULL;

	// Set current block as" used" in the free space
	// and update it to the VCB 
	return JCJC_VCB->first_freespace + block_count;
}


void init__RootDir(){

	//Find out how may blocks/bits in dir by using the getVCB_BlokCount function
	directories = malloc(sizeof(fdDir) * 20);
	// printf("dir_block_count: %d\n", fdDir * directories);

	//Use getVCB_num_bytes to find out how many bytes in dir we will need
    // int dir_num_bytes = getVCB_num_bytes(dir_block_count);
	// printf("dir_num_bytes: %d\n", dir_num_bytes);


	
	allocateFreeSpace_Bitmap(length_of_dir);

	strcpy(directories[0].d_name, "");
	// 0 -> free, 1 -> used
	directories[0].isUsed = 1;
	// 0 -> dir, 1 -> file
	directories[0].fileType = 0;

	//TODO add time later


	//----------------------------
	//Allocate the number of bytes in dir, we had previously found from dir_num_bytes
    Directory_Entry * cur_de = malloc(sizeof(Directory_Entry));
	//Set the first dir name as "."
    strcpy(cur_de->file_name, ".");
	strcpy(cur_de->filePath, "/");
	cur_de->fileSize = sizeof(fdDir);
	cur_de->fileType = 0;
	cur_de->dirUsed = 1;
	//TODO add time later

	//----------------------------
	Directory_Entry * parent_de = malloc(sizeof(Directory_Entry));
	strcpy(parent_de->file_name, "..");
	strcpy(parent_de->filePath, "/");
	parent_de->fileSize = sizeof(fdDir);
	parent_de->fileType = 0;
	parent_de->dirUsed = 1;
	//TODO add time later


	//put the init data back to the directory array we have 
	memcpy(directories[0].dirEntry[0], (char *)cur_de, JCJC_VCB->blockSize); // set the cur_de size with 512
	memcpy(directories[0].dirEntry[1], (char *)parent_de, JCJC_VCB->blockSize); // set the parent_de size with 512


	Directory_Entry * init_de = malloc(sizeof(Directory_Entry));
	strcpy(init_de->file_name, "");
	strcpy(init_de->filePath, "");
	init_de->fileSize = sizeof(fdDir);
	init_de->dirUsed = 0;
	init_de->fileSize = 0;


	//----------------------------
	int start_at_dirEntry = 2; // we will start at the dirEntry[2] first
	while(start_at_dirEntry < 10){
		memcpy(directories[0].dirEntry[start_at_dirEntry ], (char *)init_de, JCJC_VCB->blockSize);
		start_at_dirEntry ++;
	}

	//init the directory
	
	for(int i=1; i<20; i++) {// 20 should be enough
		strcpy(directories[i].d_name, "");
		directories[i].fileType = 0;
		directories[i].isUsed = 0;
		directories[i].current_location = 0;
		for(int j=0; j<10; j++){
			memcpy(directories[i].dirEntry[j], (char *)init_de, JCJC_VCB->blockSize);
		}
    }

	LBAwrite((char *)directories, length_of_dir, JCJC_VCB->location_RootDirectory);

	for(int i = freespace + 2; i < length_of_dir; i++){
		setBitFree(i,freespace);
	}

	free(parent_de);
	free(cur_de);
	free(init_de);
	parent_de = NULL;
	cur_de = NULL;
    init_de = NULL;


	
	
}