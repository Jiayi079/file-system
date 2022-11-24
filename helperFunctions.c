/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: helperFunctions.c
*
* Description: 
*	This file contains helper functions needed to execute commands
*   such as in the directory entries, VCB, freespace, etc.
*
**************************************************************/

#include "mfs.h"
#include "fsLow.h"
#include "helperFunctions.h"

// Variables for allocating space in bitmap
uint64_t targetIndex = 0, targetBit = 0;

// Function to check the bits stored in our bitmap
int checkBit(uint64_t indexOfBlock, int *bitmap)
{
    //By dividing the block index by the size of 8 bits (= 1 byte),
    //init the starting bit index and bit position to check
    targetIndex = indexOfBlock / (sizeof(int) * 8);
    targetBit = indexOfBlock % (sizeof(int) * 8);

    //Checks if the current bit is marked as free or used 
    return (bitmap[targetIndex] & (SPACE_IN_USED << targetBit)) != SPACE_IS_FREE; // 0 -> false
}

//Function to set the passed-in bit as USED in our bitmap
int setBitUsed(uint64_t indexOfBlock, int *bitmap)
{
    //Check if the current bit in the block is marked USED
    //if so, print error message
    if (checkBit(indexOfBlock, bitmap) == SPACE_IN_USED)
    {   
        printf("This bit is already marked used\n");
        return -1;
    }

    // Set the current bit postion as used in the bitmap
    // printf("bit at %ld is set to USED", indexOfBlock);
    bitmap[targetIndex] |= (SPACE_IN_USED << targetBit);
    return 0;
}

//Function to set the passed-in bit as FREE in our bitmap
int setBitFree(uint64_t indexOfBlock, int *bitmap)
{
    //Check if the current bit in the block is marked FREE
    //if so, print error message
    if (checkBit(indexOfBlock, bitmap) == SPACE_IS_FREE)
    { 
        printf("This bit is already marked free\n");
        return -1;
    }

    //Set the current bit postion as free in the bitmap
    bitmap[targetIndex] &= ~(SPACE_IN_USED << targetBit);
    return 0;
}





