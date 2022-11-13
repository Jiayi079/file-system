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
 * Description: This file contains the two functions of
 *               getting a current working directory(CWD)
 *               and setting a new CWD
 *
 **************************************************************/

#include "mfs.h"

#define MAX_ENTRIES_NUMBER 8


fdDir * get_dir_entry(struct fs_diriteminfo *entry){
    if(entry->fileType == 0){
        return NULL;
    }

    //set a buffer to read the directory entry using LBAread()
    int fdDir_size = sizeof(fdDir);
    uint64_t fdDir_block_count = getBlockCount(fdDir_size);
    char * read_buffer = malloc(fdDir_block_count * JCJC_VCB->blockSize);

    if(read_buffer != NULL){
        LBAread(read_buffer, fdDir_block_count, entry->entry_StartLocation);
    }else{
        printf("malloc read_buffer failed!\n");
        return NULL;
    }

    fdDir * ret_dir = malloc(fdDir_size);

    if(ret_dir != NULL){
        memcpy(ret_dir, read_buffer, fdDir_size);
    }else{
        printf("malloc ret_dir failed!");
        return NULL;
    }

    return ret_dir;
}



char * get_path_last_slash(char * path){
    //to find the last slash in the path using strrchr()
    char slash = '/';
    char * last_slash = strrchr(path, slash);
    
    int cut_index = last_slash - path;

    if(last_slash == NULL){
        cut_index = 0;
    }

    ldprintf("cut Index: %d", cut_index);

    // start prepare the new ptr to replace and return
    char * path_before_last_slash = malloc(cut_index + 1);
    if(path_before_last_slash == NULL){
        eprintf("malloc path_before_last_slash failed!");
        return NULL;
    }

    char * left_path = malloc(strlen(path) - cut_index);

    if(path_before_last_slash == NULL){
        eprintf("malloc left_path failed!");
        return NULL;
    }

    if(last_slash != NULL){
        strncpy(path_before_last_slash, path, cut_index);
        path_before_last_slash[cut_index] ='\0';
        strcpy(left_path, last_slash + 1);
    }else{
        strcpy(path_before_last_slash, ".");
        strcpy(left_path, path);
    }

    // put the path back to the original path
    strcpy(path, path_before_last_slash);

    printf("path before last slash is %s", path);
    printf("the left path is %s\n", left_path);

    return left_path;
}

int updateByLBAwrite(void *fdDir, uint64_t length, uint64_t startingPosition)
{
    // set up a clean buffer to copy data
    uint64_t blockCount = getBlockCount(length);
    uint64_t fullBlockSize = blockCount * JCJC_VCB->blockSize;
    char *buffer = malloc(fullBlockSize);
    if (buffer == NULL)
    {
        printf("malloc() on buffer failed\n");
        return -1;
    }

    memset(buffer, 0, fullBlockSize);

    // copy the data and then write using LBAwrite()
    memcpy(buffer, fdDir, length);
    LBAwrite(buffer, blockCount, startingPosition);

    free(buffer);
    buffer = NULL;
    return 0;
}
