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

//Function to get the block count in the VCB
unsigned int getVCB_BlockCount(uint64_t bl_number)
{
    int result = bl_number / JCJC_VCB -> blockSize;
    if (bl_number % JCJC_VCB -> blockSize > 0)
    {
        result++;
    }

    return result;
}

int checkBit(uint64_t block_index, int * freespace)
{
    return (freespace[block_index / 8] & (1 << block_index % 8)) != 0;
}

void setBitUsed(uint64_t block_index, int * freespace)
{
    freespace[block_index / 8] |= (1 << block_index % 8);
}

void setBitFree(uint64_t block_index, int * freespace)
{
    freespace[block_index / 8] &= ~(1 << block_index % 8);
}