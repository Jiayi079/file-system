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
        printf("malloc ret_dir failed!\n");
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
        eprintf("malloc path_before_last_slash failed!\n");
        return NULL;
    }

    char * left_path = malloc(strlen(path) - cut_index);

    if(path_before_last_slash == NULL){
        eprintf("malloc left_path failed!\n");
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

fdDir * get_dir_path(char * name){
    fdDir * get_dir = malloc(sizeof(fdDir));
    int fddir_size = sizeof(fdDir);

    if(get_dir != NULL){
        memcpy(get_dir, fs_CWD, fddir_size);
    }else{
        eprintf("malloc get_dir failed!\n");
        return NULL;
    }

    // try to copy name to avoid modifying it using strtok()
    char * copy_name = malloc(strlen(name) + 1);

    if(copy_name != NULL){
        strcpy(copy_name, name);
    }else{
        eprintf("malloc copy_name failed!\n");
        return NULL;
    }

    // then we will split the string by delimeter
    char delimeter = "/";
    char * token = strtok(copy_name, delimeter);


    //and we use a whild loop to check the whole list to find the directory
    while(token!= NULL){

        //if token is "." or empty it will means this is our cur dir
        //if not "." or empty it will keep finding the next
        if(strcmp(token, ".") != 0 ||strcmp(token, "") != 0){
            int  j = 0;

            for(int i=1; i < MAX_ENTRIES_NUMBER; i++){
                // to make sure is using space, a directory and name match
                if(get_dir->dirEntry[i].isFreeOrUsed == SPACE_IN_USED &&
                   get_dir->dirEntry[i].fileType == DIR_TYPE &&
                   strcmp(get_dir->dirEntry[i].d_name, token) == 0)
                   {
                    free(get_dir);
                    get_dir = get_dir_entry(get_dir->dirEntry + i);
                    j = i;
                    break;
                   }
                   j = i;
            }

            //if we didn't find the directory, that means we have fail to find
            if(j == MAX_ENTRIES_NUMBER){
                return NULL;
            }
        }
        token = strtok(NULL, delimeter);
    }
    return get_dir;
}

// return the exactly directory we find
// return NULL if not found anything
fdDir * fs_opendir(const char *name)
{
    // copy the name to avoid modifying it
    char *path = malloc(strlen(name) + 1);
    if (path == NULL)
    {
        printf("[mfs.c -- fs_opendir] malloc path failed\n");
        return NULL;
    }

    strcpy(path, name);
    directories = getDirByPath(path);

    // set the entry index to 0 for fs_readDir() works
    openedDirEntryIndex = 0;

    free(path);
    path = NULL;
    return directories;
}


//to read the dirEntry list
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    int check_de_index = openedDirEntryIndex;

    while(check_de_index < MAX_ENTRIES_NUMBER){
        if(dirp->dirEntry[check_de_index].isFreeOrUsed == SPACE_IN_USED){
            openedDirEntryIndex = check_de_index + 1;
            return dirp->dirEntry + check_de_index;
        }
        check_de_index++;
    }
    return NULL;

}

// to close the directory and free the memory we have allocated
int fs_closedir(fdDir *dirp){
    free(dirp);
    return 0;

}