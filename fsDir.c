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
#include "bitmap.c"

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
        return 0;
    }




    int parent_dirIndex = -1; // set become -1 to find the parent dir index in directories array
    Directory_Entry * parent;
    // loop to find the previous directory's location
    int i = 0;
    while (i < 20)
    {
        parent = (Directory_Entry *)directories[i].dirEntry[0];
        if (strcmp(parent->filePath, cwd) == 0) // successfully find path
        {
            parent_dirIndex = i;
            break;
        }
        i++;
    }

    if (parent_dirIndex == -1)
    {
        printf("[fsDir.c --- fs_mkdir()] Doesn't have parent path\n");
        exit(-1);
    }





    int finish_setting = 0;
    Directory_Entry * child;
    // make file directory to struct
    int j = 2; // child's directory entre start searching location
    while (j < 8)
    {
        child = (Directory_Entry *)directories[parent_dirIndex].dirEntries[j];
        if (strlen(child->filePath) == 0) // empty, can store into it
        {
            strcpy(child->file_name, pathname);
            strcpy(child->filePath, absolutePath);
            child->fileSize = sizeof(fdDir);
            child->fileType = 0;
            // haven't initialized dir_location
            // haven't initialized time yet
            memcpy(directories[parent_dirIndex].dirEntry[j], (char *)child, 512);
            finish_setting = 1;
            break;
        }
        j++;
    }

    if (finish_setting != 1)
    {
        printf("[fsDir.c --- fs_mkdir()] Doesn't have enough space for child\n");
        exit(-1);
    }

    


    return 0;
}

int fs_rmdir(const char *pathname)
{

    return 0;
}