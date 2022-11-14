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
#define MAX_NAME_LENGTH 256

// The mkdir() function creates a new, empty directory with name filename
// return 0  -> successfully
// return -1 -> failed
int fs_mkdir(const char *pathname, mode_t mode)
{

    // make copy and use that for substrings
    char *path_before_last_slash = malloc(strlen(pathname) + 1);

    if (path_before_last_slash != NULL)
    {
        strcpy(path_before_last_slash, pathname);
    }
    else
    {
        eprintf("malloc path_before_last_slash failed!");
        exit(-1);
    }

    // substrings and get the left patt as new directory name
    char *new_dir_name = get_path_last_slash(path_before_last_slash);
    if (strcmp(new_dir_name, "") == 0)
    {
        printf("there no director name\n");
        return -1;
    }
    if (new_dir_name == NULL)
    {
        printf("get_path_last_slash failed!");
        return -1;
    }

    // to get the dir ptr
    fdDir *parent_dir = get_dir_path(path_before_last_slash);

    if (parent_dir == NULL)
    {
        printf("%s is not exisited from cwd\n", path_before_last_slash);

        free(path_before_last_slash);
        path_before_last_slash = NULL;
        free(new_dir_name);
        new_dir_name = NULL;
        return -1;
    }

    // create direcotory
    // check if the dirEntryCount is less than the maximum number of ammount entries -> 8
    if (parent_dir->dirEntryCount < 8)
    {
        // can hold dirEntry
        // check if the space is free or not
        // check if the name already exists
        for (int i = 0; i < 8; i++)
        {
            if (parent_dir->dirEntry[i].isFreeOrUsed == 1 &&
                strcmp(parent_dir->dirEntry[i].d_name, new_dir_name) == 0)
            {
                printf("[mfs.c -- mkdir] name already exsist in directory\n");

                // avoid memory leak
                free(path_before_last_slash);
                path_before_last_slash = NULL;
                free(new_dir_name);
                new_dir_name = NULL;
                free(parent_dir);
                parent_dir = NULL;
                return -1;
            }
        }

        printf("creating new directory '%s' successfully\n", new_dir_name);

        // makeing new directory

        fdDir *creatDir;

        if (creatDir == NULL)
        {
            printf("malloc() creatDir failed\n");
            return NULL;
        }

        memset(creatDir, 0, sizeof(fdDir));

        // initialize the directory and allocate the space for it
        uint64_t dirBlockCount = getBlockCount(sizeof(fdDir));
        int findFreeSpace = allocateFreespace(dirBlockCount);
        if (findFreeSpace <= 0)
        {
            printf("[mfs.c -- mkdir] don't have enough free space\n");
            return -1;
        }
        creatDir->directoryStartLocation = findFreeSpace;
        creatDir->d_reclen = sizeof(fdDir);
        creatDir->dirEntryCount = 2;

        if (strlen(new_dir_name) > (MAX_NAME_LENGTH - 1))
        {
            char *name = malloc(MAX_NAME_LENGTH);
            if (name == NULL)
            {
                printf("[mfs.c -- mkdir] malloc shorName failed\n");
                return NULL;
            }

            // make sure it only contains one less than the max for null terminator
            strncpy(name, new_dir_name, MAX_NAME_LENGTH - 1);
            name[MAX_NAME_LENGTH - 1] = '\0';
            strcpy(creatDir->d_name, name);

            free(name);
            name = NULL;
        }

        // initialize creatDir
        strcpy(creatDir->dirEntry[0].d_name, ".");
        creatDir->dirEntry[0].fileType = 0;
        creatDir->dirEntry[0].isFreeOrUsed = 1;
        creatDir->dirEntry[0].entry_StartLocation = findFreeSpace;
        creatDir->dirEntry[0].d_reclen = sizeof(struct fs_diriteminfo);
        creatDir->dirEntry[0].fileSize = sizeof(fdDir);

        if (parent_dir->dirEntry == NULL)
        { // if parent is NULL, this is a root directory
            memcpy(creatDir->dirEntry + 1, creatDir->dirEntry, sizeof(struct fs_diriteminfo));
        }
        else
        { // if parent is not NULL, we copy the parent into this entry
            memcpy(creatDir->dirEntry + 1, creatDir, sizeof(struct fs_diriteminfo));
        }

        // parent directory needs to be renamed to ..
        strcpy(creatDir->dirEntry[1].d_name, "..");

        for (int i = 2; i < 8; i++)
        {
            creatDir->dirEntry[i].isFreeOrUsed = 0;
        }

        // finding the available free space starting location
        for (int i = 2; i < 8; i++)
        {
            if (parent_dir->dirEntry[i].isFreeOrUsed == 0)
            {
                // if dirEntry is free, put the data in
                parent_dir->dirEntryCount++;
                parent_dir->dirEntry[i].d_reclen = sizeof(struct fs_diriteminfo);
                parent_dir->dirEntry[i].fileType = 0; // 0 -> dir
                parent_dir->dirEntry[i].entry_StartLocation = creatDir->directoryStartLocation;
                parent_dir->dirEntry[i].isFreeOrUsed = 1;
                parent_dir->dirEntry[i].fileSize = sizeof(fdDir);
                strcpy(parent_dir->dirEntry[i].d_name, creatDir->d_name);

                // update creatDir directory
                updateByLBAwrite(creatDir, creatDir->d_reclen, creatDir->directoryStartLocation);
                if (fs_CWD != NULL && creatDir->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, creatDir, sizeof(fdDir));
                }

                // update parent_dir directory
                updateByLBAwrite(parent_dir, parent_dir->d_reclen, parent_dir->directoryStartLocation);
                if (fs_CWD != NULL && parent_dir->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, parent_dir, sizeof(fdDir));
                }

                break; // break out of loop
            }
        }
    }
    else
    {
        printf("Directory doesn't have enough space\n");
        free(path_before_last_slash);
        path_before_last_slash = NULL;
        free(new_dir_name);
        new_dir_name = NULL;
        free(parent_dir);
        parent_dir = NULL;
        return -1;
    }

    free(path_before_last_slash);
    path_before_last_slash = NULL;
    free(new_dir_name);
    new_dir_name = NULL;
    free(parent_dir);
    parent_dir = NULL;
    return 0;
}



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

int fs_isFile(char *path)
{
    // keep a copy of fsCWD and openedDir
    fdDir *fd_Dir = fs_CWD;

    // replace cwd by openedDir if a directory is open
    int checkDirOpened = 0;
    if (directories != NULL)
    {
        checkDirOpened = 1;
        fs_CWD = directories;
    }

    // make a copy and substring before the last slash
    char *pathBeforeLastSlash = malloc(strlen(path) + 1);
    if (pathBeforeLastSlash == NULL)
    {
        printf("[mfs.c -- fs_isFile] malloc pathBeforeLastSlash failed\n");
        return -1;
    }

    strcpy(pathBeforeLastSlash, path);
    char * filename = getPathByLastSlash(pathBeforeLastSlash);

    // find the directory that is expected for holding that file
    fdDir * new_dir = getDirByPath(pathBeforeLastSlash);

    int result = 0;

    // if the path is not even in a directory, then we don't need to check anymore
    if (new_dir != NULL)
    {
        // check if the item is inside this directory
        for (int i = 2; i < 8; i++)
        { // make sure that is a used space, a directory and name matched
            if (new_dir->dirEntry[i].isFreeOrUsed == 1 &&
                new_dir->dirEntry[i].fileType == 1 &&
                strcmp(new_dir->dirEntry[i].d_name, filename) == 0)
            {
                result = 1;
            }
        }
    }

    // make sure we reset and also free the retPtr
    if (checkDirOpened)
    {
        fs_CWD = fd_Dir;
    }

    free(new_dir);
    new_dir = NULL;
    free(pathBeforeLastSlash);
    pathBeforeLastSlash = NULL;
    free(filename);
    filename = NULL;

    return result;
}

// to load the status of the file in the opened directory
int fs_stat(const char *path, struct fs_stat *buf)
{
    // because it is utilizing an existing directory, it shouldn't fail.
    for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
    {
        // check if the driectories's location is used
        if (directories->dirEntry[i].isFreeOrUsed == 1 &&
        // also check if the directory is what we are looking for
            strcmp(directories->dirEntry[i].d_name, path) == 0)
        {
            // set up the data to fs_stat struct
            buf->st_blksize = JCJC_VCB->blockSize;
            buf->st_size = directories->dirEntry[i].fileSize;
            buf->st_blocks = getBlockCount(buf->st_size);
            // when we finish setting up the data, finish this function
            return 0;
        }
    }

    // return -1 if only if didn't find the path or if the directory all full
    return -1;
}


int fs_isDir(char * pathname){
    fdDir * cp_cwd = fs_CWD;
    printf("the original fs cwd is: %X", fs_CWD);

    //replace the cwd by our open_dir if that is opened
    int dir_open = 0;
    if(directories == NULL){
        printf("[mfs.c -- fs_isDir] directories is NULL\n");
        return -1;
    }else{
        dir_open = 1;
        fs_CWD = directories;
    }

    //use get_dir_path to check DIR_TYPE while running
    fdDir * tempPtr = get_dir_path(pathname);
    int result = 0;
    if(tempPtr == NULL){
        printf("[mfs.c -- fs_isDir] tempPtr is NULL\n");
        return -1;
    }else{
        result = 1;
    }


    //rest athe fs_CWD
    fs_CWD = cp_cwd;
    
    //free the retPtr
    free(tempPtr);
    tempPtr = NULL;
    return result;
}




