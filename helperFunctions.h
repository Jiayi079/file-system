/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: helperFunctions.h
*
* Description: 
*	This file contains helper functions needed to execute commands
*   such as checking and setting the bits in our bitmap, getting
*   the number of bytes needed, and allocating free space.
*
**************************************************************/


#include "mfs.h"
#include "bitmap.c"

unsigned int getVCB_BlockCount(uint64_t bl_number);

int allocateFreeSpace_Bitmap(int block_count_needed, int index);

unsigned int getVCB_num_bytes(uint64_t block_count);

int checkBit(uint64_t block_index, int * freespace);

void setBitUsed(uint64_t block_index, int * freespace);

void setBitFree(uint64_t block_index, int * freespace);