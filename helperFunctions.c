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
int checkBit(uint64_t indexOfBlock, int * freespace)
{
    //By dividing the block index by the size of 8 bits (= 1 byte),
    //init the starting bit index and bit position to check
    targetIndex = indexOfBlock / (sizeof(int) * 8);
    targetBit = indexOfBlock % (sizeof(int) * 8);

    //Checks if the current bit is marked as free or used 
    return (freespace[targetIndex] & (SPACE_IN_USED << targetBit)) != SPACE_IS_FREE; // 0 -> false
}

//Function to set the passed-in bit as USED in our bitmap
int setBitUsed(uint64_t indexOfBlock, int * freespace)
{
    //Check if the current bit in the block is marked USED
    //if so, print error message
    if (checkBit(indexOfBlock, freespace) == SPACE_IN_USED)
    {   
        printf("This bit is already marked used\n");
        return -1;
    }

    // Set the current bit postion as used in the bitmap
    // printf("bit at %ld is set to USED", indexOfBlock);
    freespace[targetIndex] |= (SPACE_IN_USED << targetBit);
    return 0;
}

//Function to set the passed-in bit as FREE in our bitmap
int setBitFree(uint64_t indexOfBlock, int * freespace)
{
    //Check if the current bit in the block is marked FREE
    //if so, print error message
    if (checkBit(indexOfBlock, freespace) == SPACE_IS_FREE)
    { 
        printf("This bit is already marked free\n");
        return -1;
    }

    //Set the current bit postion as free in the bitmap
    freespace[targetIndex] &= ~(SPACE_IN_USED << targetBit);
    return 0;
}

//Function to find and allocate free space in our bitmap by 
//using contigous free blocks
uint64_t allocateFreeSpace_Bitmap(uint64_t block_ToBeAllocated)
{
    //If the block to be allocated is 0 (VCB), then print
    //error message 
    if (block_ToBeAllocated <= 0)
    {
        printf("invalid arg in allocateFreeSpace_Bit");
        return -1;
    }

    uint64_t num_OfBlocksRequested = 0;

    //Using the first free block index in our VCB can an 
    //efficient way of allocating many files in the volume
    for (uint64_t b_index = JCJC_VCB->current_FreeBlockIndex; b_index < JCJC_VCB->numberOfBlocks; b_index++)
    {
        //Check to see if the bitmap has any available free space 
        if (checkBit(b_index, freespace) == SPACE_IS_FREE)
        {
            num_OfBlocksRequested++;
            //Let the number of blocks requested be the number of 
            //blocks needed to be allocated in the volume 
            if (num_OfBlocksRequested == block_ToBeAllocated)
            {
                //In this loop, we set the bit of the allocated blocks 
                //as used
                for (uint64_t next_Block = 0; next_Block < num_OfBlocksRequested; next_Block++)
                {
                    // handle error when setBitUsed get in errors
                    if (setBitUsed(b_index - next_Block, freespace) != 0)
                    {
                        printf("Failed to set bit used at block: %ld", next_Block);

                        //If are no more availabe blocks left at next_Block, then
                        //go back to the previous block and mark it as free
                        for (next_Block--; next_Block >= 0; next_Block--)
                        {
                            setBitFree(b_index - next_Block, freespace);
                        }
                        return -1;
                    }
                }

                //Check if the first free block in our VCB is marked USED
                //if so, then we need to update it in our VCB
                if (checkBit(JCJC_VCB->current_FreeBlockIndex, freespace) == SPACE_IN_USED)
                {
                    //Using the last occupied block in our VCB, we check to see if
                    //there are any available free space, if so, then LBAwrite that 
                    //block into our VCB
                    for (uint64_t k = b_index + 1; k < JCJC_VCB->numberOfBlocks; k++)
                    {
                        if (checkBit(k, freespace) == SPACE_IS_FREE)
                        {
                            //Set the new first free block index and update it in JCJC_VCB
                            // printf("first free block index changes to %ld\n", k);

                            JCJC_VCB->current_FreeBlockIndex = k;

                            LBAwrtie_func(JCJC_VCB, sizeof(volume_ControlBlock), 0);

                            break;
                        }
                    }
                }

                //Since we are doing bit operations, we need to call
                //convertBitToBytes() to convert the bit blocks
                //into byte blocks to be read by the file system 
                uint64_t bytes = convertBitToBytes();

                //Write the converted number of bytes into the VCB
                LBAwrtie_func(freespace, bytes, JCJC_VCB->VCB_blockCount);

                // testing freespace on removing
                // printf("\nused block index: ");
                // for (int i = 0; i < JCJC_VCB->numberOfBlocks; i++)
                // {
                //     if (checkBit(i, freespace) == 1)
                //     {
                //         printf("%d ", i);
                //     }
                // }

                // uint64_t val = b_index - block_ToBeAllocated + 1;
                // printf("returning block index: %ld\n", val);
                return b_index - block_ToBeAllocated + 1;
            }
        }
        else
        {
            //Reset the number of blocks requested to 0 for the next operation 
            num_OfBlocksRequested = 0;
        }
    }

    //If there is not enough free space to allocate the 
    //requested blocks, then print error message
    printf("\nNOT enough space\n");
    return -1;
}

//Function to convert the bits in our VCB to bytes
//that can be read by our File System
int convertBitToBytes()
{
    uint64_t bytes = JCJC_VCB->numberOfBlocks / 8;
    if (JCJC_VCB->numberOfBlocks % 8 > 0)
    {
        bytes++;
    }
    return bytes;
}