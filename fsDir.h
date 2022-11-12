/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: fsDir.h
*
* Description: This file contains functions on getting a
*               specifc directory entry index as well as
*               getting the index of an unused directory
*
*
*
**************************************************************/

#include "mfs.h"
#include "mfs.h"




int getNotInUseDir();
int getDirIndex(Directory_Entry * Dir, char *path);