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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mfs.h"
#include "fsLow.h"
#include "fsDir.h"
#include "bitmap.c"
#include "fsInit.c"

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
    // if (current_dirIndex != -1)
    // {
    //     printf("[fsDir.c --- fs_mkdir()] Directory already exists\n");
    //     return 0;
    // }






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
        child = (Directory_Entry *)directories[parent_dirIndex].dirEntry[j];
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



    // find empty directory location
    int empty_dir_location = getNotInUseDir();

    if (empty_dir_location == -1)
    {
        printf("[fsDir.c --- fs_mk] Doesn't have empty directory\n");
        return -1;
    }

    strcpy(directories[empty_dir_location].d_name, pathname);
    directories[empty_dir_location].isUsed = 1;
    directories[empty_dir_location].fileType = 0;

    Directory_Entry * current_DE;
    current_DE = malloc(sizeof(Directory_Entry));

    strcpy(current_DE->file_name, '.');
    // dir_location doesn't finish setting
    current_DE->fileType = 0;
    current_DE->fileSize = sizeof(fdDir);
    strcpy(current_DE->filePath, absolutePath);
    current_DE->dirUsed = 1;

    Directory_Entry * parent_DE;
    parent_DE = malloc(sizeof(Directory_Entry));
    strcpy(parent_DE->file_name, '..');
    parent_DE->fileType = 0;
    parent_DE->fileSize = sizeof(fdDir);
    strcpy(parent_DE->filePath, parent->filePath);
    parent_DE->dirUsed = 1;

    memcpy(directories[empty_dir_location].dirEntry[0], (char *)current_DE, 512);
    memcpy(directories[empty_dir_location].dirEntry[1], (char *)parent_DE, 512);

    int blockCount = (dir_DE_count * sizeof(Directory_Entry)) / 512 + 1;
    // TODO
    // LBAwrite((char *)directories, blockCount, ); // last paremeter for directories location, doesn't finish


    free(current_DE);
    current_DE = NULL;
    free(parent_DE);
    parent_DE = NULL;

    return 1;
}

int fs_rmdir(const char *pathname)
{

    return 0;
}

// to get our parent path 
char * get_parent_path(char * pathname)
{
    char path_name[256];
    char * token;
    char * save_ptr;

    strcpy(path_name, pathname);
    
    //get our token
    token = strtok_r(path_name, "/", &save_ptr);
    //set our partent_path_name starting with "/"
    char partent_path_name[256] = "/";
    
    int cur_token_length;
    while(token!= NULL){
        //find out our current token length
        cur_token_length = strlen(token)+1;
        //add token after our partent_path_name
        strcat(partent_path_name, token);
        //add "/" after our partent_path_name
        sttcat(partent_path_name, "/");
        token = strtok_r(NULL, "/", &save_ptr);
    }

    //now will try to get our parent path
    //set our partent_path_name's length correctly
    int ppnLength = strlen(partent_path_name) -1;

    //----------------------
    int k = 0;
    while(k<cur_token_length){
        partent_path_name[ppnLength - k] = 0;
        k++;
    }

    //if parent path is not our root directory
    // we need to delete the last "/"
    if(strlen(partent_path_name) > 1){
        partent_path_name[ppnLength - cur_token_length] = 0;
    }


    return save_ptr = partent_path_name;
}