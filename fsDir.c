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
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mfs.h"
#include "fsLow.h"
#include "fsDir.h"

char cwd[512];

// The mkdir() function creates a new, empty directory with name filename
// return 0  -> successfully
// return -1 -> failed
int fs_mkdir(const char *pathname, mode_t mode)
{
    char absolutePath[256];
    // printf("[fsDir.c --- fs_mkder]pathname: %s\n", pathname);
    if (strlen(pathname) > 0)
    {
        if (strlen(cwd) > 1) // cwd already have something
        {
            strcat(absolutePath, cwd); // copy previous cwd to path
            strcat(absolutePath, '/'); // adding slash to separate
            strcat(absolutePath, pathname);
        }
        else // cwd not a root
        {
            strcat(absolutePath, cwd);
            strcat(absolutePath, pathname);
        }
    }
    else
    {
        printf("[fsDir.c --- fs_mkdir()] Making directory failed\n");
        exit(-1);
    }

    Directory_Entry * temp;

    int current_dirIndex = getDirIndex(temp, absolutePath);

    // current_dirIndex will equal -1 if only if the directories already exist
    if (current_dirIndex != -1)
    {
        printf("[fsDir.c --- fs_mkdir()] Directory already exists\n");
        exit(-1);
    }

    int parent_dirIndex = -1; // set become -1 to find the parent dir index in directories array
    Directory_Entry * parent;
    for (int i = 0; i < 20; i++)
    {
        parent_dirIndex = (Directory_Entry *); // havent finished
        if (strcmp(parent->filePath, cwd))
    }

    int finish = 0;
    Directory_Entry * child;

    for (int i = 2; i < 8; i++)
    {
        child = (Directory_Entry *); // haven't finished
        if (strlen(child->filePath) == 0)
        {
            
        }
    }

    return 0;
}

int fs_rmdir(const char *pathname)
{

    return 0;
}
