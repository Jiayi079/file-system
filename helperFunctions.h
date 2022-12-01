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

uint64_t allocateFreeSpace_Bitmap(uint64_t block_ToBeAllocated);
int setUsed(uint64_t, int * freespace);
int setBitFree(uint64_t, int * freespace);
int checkBit(uint64_t, int * freespace);
int convertBitToBytes();