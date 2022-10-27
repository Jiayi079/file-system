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

int allocateFreeSpace(int block_count_needed)
{
    int freespace_start_location = JCJC_VCB->current_FreeBlockIndex;

    while(1)
    {
        int count = 0;
        int j, k;
        for (int i = freespace_start_location; i < JCJC_VCB->numberOfBlocks; i++)
        {
            if (checkBit(i, freespace) == 0) //finding free space
            {
                count++;
                // finding next allocated space location
                for (j = i + 1; j < JCJC_VCB->numberOfBlocks; j++)
                {
                    if (checkBit(j, freespace) != 0)
                    {
                        break; // keep j, cause j is the next allocated file's starting location
                    }
                    count++;
                }
            }
            break;
        }

        if (count >= block_count_needed)
        {
            return freespace_start_location;
        }
        else 
        {
            for (k = j; k < JCJC_VCB->numberOfBlocks; k++)
            {
                if (checkBit(k, freespace) == 0) // find the allocated file end
                {
                    count = 0; // loop to find next free space, so initial count = 0
                    freespace_start_location = k;
                    break;
                }
            }
        }
        if (k >= JCJC_VCB->numberOfBlocks)
        {
            break;
        }       
    }

    return -1; // don't have enough free space
}

unsigned int getVCB_num_bytes(uint64_t block_count)
{
    int result = block_count * JCJC_VCB->blockSize;
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
