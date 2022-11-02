/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: mfs.c
*
* Description: This file contains all the functions needed
*               to access, open, read, close, etc. for
*               all directory-related actions
*	
*
**************************************************************/

#include "mfs.h"

//Function to parse pathname of a directory from a specifc entry
fdDir * parse_DirectoryEntry(struct fs_diriteminfo * dir_entry){

    if (dir_entry -> fileType != type_isDirectory){

        return NULL;
    }

    unsigned int fd_DirBlockCount = getVCB_BlockCount(sizeof(fdDir));
    char * de_Buffer = malloc(fd_DirBlockCount * JCJC_VCB -> blockSize);

    if(de_Buffer == NULL){

        printf("Failed to malloc Directory Entry Buffer\n");
        return NULL;
    }

    fdDir * fetched_Dir = malloc(sizeof(fdDir));

    if(fetched_Dir == NULL){

        printf("Failed to malloc Fetched Directory\n");
        return NULL;

    }

    LBAread(de_Buffer, fd_DirBlockCount, dir_entry -> entry_StartLocation);
    memcpy(fetched_Dir, de_Buffer, sizeof(fdDir));


    return fetched_Dir;
}


//Function to get the current working directory(CWD)
char * fs_getcwd(char *pathname, size_t size){

    strcpy(pathname, ""); //set initial pathname for directory blank/empty string

    char * cwd_PathBuffer = malloc(size);
    if(cwd_PathBuffer == NULL){

        printf("Failed to malloc CWD Buffer\n");
        return NULL;

    }

    fdDir * dir_Copy = malloc(sizeof(fdDir));
    if(dir_Copy == NULL){

        printf("Failed to malloc Directory Copy\n");
        return NULL;

    }

    memcpy(dir_Copy, fs_CWD, sizeof(fdDir));

    //Using a while-loop, start from the current directory entry and working 
    //backwards, go back up one directory at a time until root location is
    //reached. The result will be the full path of the directory entry 
    while(dir_Copy -> directoryStartLocation != JCJC_VCB -> location_RootDirectory){

        strcpy(cwd_PathBuffer, "/");
        strcat(cwd_PathBuffer, dir_Copy -> d_name);
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);

        //Pointer for the parent directory 
        fdDir * parentDir_ptr = parse_DirectoryEntry(dir_Copy -> dir_DE_count + 1);


        free(dir_Copy);
        dir_Copy = parentDir_ptr;

    }

    //If a certain directory is empty, then put a "." to mark as root dir
    if(strcmp(pathname, "") == 0){

        strcpy(pathname, ".");

    //If a directory is not empty, then move the root directory to 
    //the front of the pathname    
    } else {

        strcpy(cwd_PathBuffer, ".");
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);
    }

    free(dir_Copy);
    free(cwd_PathBuffer);
    dir_Copy = NULL;
    cwd_PathBuffer = NULL;
    return pathname;

}
