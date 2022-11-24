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

// //Function to get the block count in the VCB
// unsigned int getVCB_BlockCount(uint64_t bl_number)
// {
//     int result = bl_number / JCJC_VCB -> blockSize;
//     if (bl_number % JCJC_VCB -> blockSize > 0)
//     {
//         result++;
//     }
//     return result;
// }



// //Function to get how many bytes are needed from our VCB
// unsigned int getVCB_num_bytes(uint64_t block_count)
// {
//     int result = block_count * JCJC_VCB -> blockSize;
//     return result;
// }

//From git pull import, removing will generate complie error 
//Will fix later 
// unsigned int getVCB_BlockCount(uint64_t bl_number);

// int allocateFreeSpace_Bitmap(int block_count_needed);

// unsigned int getVCB_num_bytes(uint64_t block_count);

// void setBitUsed(uint64_t block_index, int * freespace);

// void setBitFree(uint64_t block_index, int * freespace);


