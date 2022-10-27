#include "mfs.h"

//From git pull import, removing will generate complie error 
//Will fix later 
unsigned int getVCB_BlockCount(uint64_t bl_number);

int allocateFreeSpace_Bitmap(int block_count_needed);

unsigned int getVCB_num_bytes(uint64_t block_count);

int checkBit(uint64_t block_index, int * freespace);

void setBitUsed(uint64_t block_index, int * freespace);

void setBitFree(uint64_t block_index, int * freespace);