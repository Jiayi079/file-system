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
#include "helperFunctions.h"

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


//Function to parse pathname of a directory from a specifc entry
fdDir * parseEntry(struct Directory_Entry *entry)
{
    //Check if passed-in directory entry is     
    //of type Directory, if not, print error
    if (entry->fileType != DIR_TYPE)
    {
        return NULL;
    }

    //Get the block num_BlocksToRelease of this directory entry, and by 
    //using the sizeOf, malloc a temp temp_LBABuffer for entry
    unsigned int fdDir_block_count = getVCB_BlockCount(sizeof(fdDir));
    char *buffer = malloc(fdDir_block_count * JCJC_VCB->blockSize);
    if (buffer == NULL)
    {
        printf("malloc() on buffer");
        return NULL;
    }

    fdDir *tempDir = malloc(sizeof(fdDir));
    if (tempDir == NULL)
    {
        printf("malloc() on tempDir");
        return NULL;
    }

    //Using LBAread, update the directory entry block_StartPos location
    //and copy the pure_PathCopy from de_Buffer to our relative pathname pointer
    LBAread(buffer, fdDir_block_count, entry->dir_Location);
    memcpy(tempDir, buffer, sizeof(fdDir));

    return tempDir;
}


//Function to get the full pathname before the last "/"
//of the current directory 
char * get_path_last_slash(char * path)
{
    //To find the last slash in the pure_PathCopy, we used strrchr()
    char slash = '/';
    char *last_slash = strrchr(path, slash);

    int cut_index = last_slash - path;

    //If there is nothing found after the last slash
    //then set the splice_index to 0
    if (last_slash == NULL)
    {
        cut_index = 0;
    }

    //Display the index of where the path was spliced
    // printf("Path spliced at index: %d\n", cut_index);

    //Malloc a path copy before the last slash into memory
    char *path_before_last_slash = malloc(cut_index + 1);

    if (path_before_last_slash == NULL)
    {
        printf("Failed to malloc path_before_last_slash\n");
        return NULL;
    }

    //Malloc a path copy before the splice index (lefthalf) 
    //into memory
    char *left_path = malloc(strlen(path) - cut_index);

    if (path_before_last_slash == NULL)
    {
        printf("Failed to malloc lefthalf_Path\n");
        return NULL;
    }

    //While the directory at the last slash is not empty (NULL),
    //copy the path from the splice index and lefthalf,
    //into our pure_PathCopy buffer
    if (last_slash != NULL)
    {

        strncpy(path_before_last_slash, path, cut_index);
        path_before_last_slash[cut_index] = '\0';
        strcpy(left_path, last_slash + 1);

    }

    else

    {
        //If the directory is empty, then set it as root 
        strcpy(path_before_last_slash, ".");
        strcpy(left_path, path);

    }

    //Copy the path from path_before_last_slash back into the original pure_PathCopy
    strcpy(path, path_before_last_slash);

    //Display the path before and after the slash
    // printf("pure_PathCopy before last slash is: %s\n", path);
    // printf("The lefthalf pure_PathCopy is: %s\n", left_path);

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

//Function to get the full pure_PathCopy of a directory 
fdDir * parsePath(char *name)
{
    //Malloc a pointer for the absolute pure_PathCopy for 
    //a directory in current working directory (CWD)
    // printf("name: %s\n", name);
    int fddir_size = sizeof(fdDir);
    fdDir *absolute_DirPath = malloc(fddir_size);

    if (absolute_DirPath != NULL)
    {
        //Copy the current fs_CWD of the file system
        memcpy(absolute_DirPath, fs_CWD, fddir_size);
    }
    else
    {
        printf("Failed to malloc absolute_DirPath\n");
        return NULL;
    }

    //Make a pure copy of the DE_pathname, so that the 
    //original pathname will not be modified
    char * pure_DirPathCopy = malloc(strlen(name) + 1);

    if (pure_DirPathCopy != NULL)
    {
        strcpy(pure_DirPathCopy, name);
    }
    else
    {
        printf("malloc pure_DirPathCopy failed!\n");
        return NULL;
    }

    // We will use the delimeter "/", to split the strings
    // and represent new directories with the delim as well
    char * delimeter = "/";
    char * token = strtok(pure_DirPathCopy, delimeter);

    int j = 0;

    while (token != NULL)
    {
        // printf("token: %s\n", token);
        //Check if current path_Token is not root "." or not empty ""
        if (strcmp(token, ".") != 0 || strcmp(token, "") != 0)
        {
            //Loop through the directory entries in dir_DE_count
            //and check if any of the dir entries matches with the 
            //string tokens from strtok()


            for (int i = 1; i < MAX_ENTRIES_NUMBER; i++)
            {
                // if (absolute_DirPath->dirEntry[i].dirUsed == SPACE_IN_USED &&
                //     absolute_DirPath->dirEntry[i].fileType == FILE_TYPE &&
                //     strcmp(absolute_DirPath->dirEntry[i].file_name, token) == 0)
                // {
                //     printf("Failed to find directory, since name is: %s\n", token);
                //     break;
                // }
                // We need to make sure that each dir entry is allocated 
                // in a used space in the bitmap, as well as of type directory
                // and the entry must have a matching name
                // printf("absolute_DirPath fileType: %d, file_name: %s\n", absolute_DirPath->dirEntry[i].fileType, 
                //     absolute_DirPath->dirEntry[i].file_name);
                if (absolute_DirPath->dirEntry[i].dirUsed == SPACE_IN_USED &&
                    // absolute_DirPath->dirEntry[i].fileType == DIR_TYPE &&
                    strcmp(absolute_DirPath->dirEntry[i].file_name, token) == 0)
                {
                    printf("find: %s\n", absolute_DirPath->dirEntry[i].file_name);
                    //Free the pointer temp_LBABuffer and parse the directory entry's full
                    //pathname by calling the parse_DirectoryEntry function
                    free(absolute_DirPath);
                    absolute_DirPath = parseEntry(absolute_DirPath->dirEntry + i);
                    j = i;
                    break;
                }
                // j = i;
            }

            //If we looped through all the stored directories and did
            //not find the specified directroty, then print error message
            if (j == MAX_ENTRIES_NUMBER)
            {
                printf("Failed to find the specified directory\n");
                return NULL;
            }
            token = strtok(NULL, delimeter);
        }
        else
        {
            token = strtok(NULL, delimeter);
        }
    }//End while loop

    return absolute_DirPath;
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
    directories = get_dir_path(path);

    // set the entry index to 0 for fs_readDir() works
    openedDirEntryIndex = 0;

    free(path);
    path = NULL;
    return directories;
}


//Function to read the directory entries list
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{
    //Set the current dir entry index to the amount of 
    //currently opened dir entries 
    int check_de_index = openedDirEntryIndex;

    //While the current index is less than max num entries,
    //set each entry with an index number 
    while (check_de_index < MAX_ENTRIES_NUMBER)
    {
        if (dirp->dirEntry[check_de_index].dirUsed == SPACE_IN_USED)
        {
            openedDirEntryIndex = check_de_index + 1;
            return dirp->dirEntry + check_de_index;
        }
        check_de_index++;
    }
    //If there are no more entries to index, then return NULL
    return NULL;
}

// to close the directory and free the memory we have allocated
int fs_closedir(fdDir *dirp){
    free(dirp);
    return 0;
}

//Function to check if the passed-in entry
//is of type File 
//TODO CHECK
int fs_isFile(char * filename)
{
    //We keep tempCWD for later use to copy back to the fs_CWD,
    //since we have to set the root directory as fs_CWD
    // printf("check isFile filename: %s\n", filename);
    fdDir *tempCWD = fs_CWD;

    //Initialize an open directory indicator, and if the root directory
    //contains any data in it, then the directory will be marked as open
    int dirIsOpened = 0;
    if (rootDir != NULL)
    {
        dirIsOpened = 1;
        fs_CWD = rootDir;
    }

    //Make a copy of the file's path and substring the path with a 
    //slash by calling parsePath() function
    char *pathExculdeLastSlash = malloc(strlen(filename) + 1);
    if (pathExculdeLastSlash != NULL)
    {
        strcpy(pathExculdeLastSlash, filename);
    }
    else
    {
        printf("[mfs.c -- fs_isFile] malloc pathExculdeLastSlash failed\n");
        return -1;
    }

    char *getfilename = get_path_last_slash(pathExculdeLastSlash);

    //Find the directory that contains this specific file by 
    //calling yhe parsePath() function
    fdDir *filePathDir = parsePath(pathExculdeLastSlash);

    //If the file doesn't exist, this will automatically 
    //not run this if statement
    if (filePathDir != NULL)
    {
        //Check if the file item is inside this directory
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            //Check if the directory entry is marked as used, 
            //is of type file, and if the directory name matches
            //the expected dir name
            // printf("filePathDir->dirEntry[i].fileType = %d\n", filePathDir->dirEntry[i].fileType);
            // printf("filePathDir->dirEntry[i].file_name = %s\n", filePathDir->dirEntry[i].file_name);
            if (filePathDir->dirEntry[i].dirUsed == SPACE_IN_USED &&
                filePathDir->dirEntry[i].fileType == FILE_TYPE &&
                strcmp(filePathDir->dirEntry[i].file_name, getfilename) == 0)
            {

                //Set the directory pointer from tempCWD back to fs_CWD
                if (dirIsOpened)
                {
                    fs_CWD = tempCWD;
                }

                //Free our filePathDir and pointers
                //and set all to NULL for next operation
                free(filePathDir);
                filePathDir = NULL;
                free(pathExculdeLastSlash);
                pathExculdeLastSlash = NULL;
                free(getfilename);
                getfilename = NULL;

                return 1;
            }
        }
    }

    //Set the directory pointer from tempCWD back to fs_CWD
    if (dirIsOpened)
    {
        fs_CWD = tempCWD;
    }

    //Free our filePathDir and pointers
    //and set all to NULL for next operation
    free(filePathDir);
    filePathDir = NULL;
    free(pathExculdeLastSlash);
    pathExculdeLastSlash = NULL;
    free(getfilename);
    getfilename = NULL;

    return 0;
}

//Funcion to load the status of a file in the opened directory
int fs_stat(const char *path, struct fs_stat *buf)
{
    //In this loop, first check if the current directory entry is marked used
    //and if that specific directory exsits
    for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
    {
        if (rootDir->dirEntry[i].dirUsed == SPACE_IN_USED &&
            strcmp(rootDir->dirEntry[i].file_name, path) == 0)
        {
            //If specified directory exists, then set the 
            //file data into our fs_stat struct 
            buf->st_blksize = JCJC_VCB->blockSize;
            buf->st_size = rootDir->dirEntry[i].fileSize;
            buf->st_blocks = getVCB_BlockCount(buf->st_size);
            buf->st_createtime = rootDir->dirEntry[i].create_time;
            buf->st_accesstime = rootDir->dirEntry[i].last_access_time;
            buf->st_modtime = rootDir->dirEntry[i].modified_time;
            // when we finish setting up the data, finish this function
            return 0;
        }
    }

    // return -1 if it didn't find the directory in pure_PathCopy 
    //or if the directory is already full
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


    

//Function to get the current working directory(CWD)
char * fs_getcwd(char * pathname, size_t size){

    strcpy(pathname, ""); //set initial pathname for directory blank/empty string

    //Malloc a buffer that will hold the CWD path
    char * cwd_PathBuffer = malloc(size);
    if(cwd_PathBuffer == NULL){

        printf("Failed to malloc CWD Buffer\n");
        return NULL;

    }

    //Malloc a copy of the directory, so we can loop through it
    //when we need to parse the path
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

        //Using string copy and string concatenation, parse the path
        //of the dir entry, and seperate each with a "/"
        strcpy(cwd_PathBuffer, "/");
        strcat(cwd_PathBuffer, dir_Copy -> d_name);
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);

        //Pointer for the parent directory 
        fdDir * parentDir_ptr = get_dir_entry(dir_Copy -> dirEntry + 1);

        //Free our buffer copy of the original directory
        //and set the parent directory to this fs_CWD
        free(dir_Copy);
        dir_Copy = parentDir_ptr;

    }

    //If a certain directory is empty, then put a "." to mark as root dir
    if(strcmp(pathname, "") == 0){

        strcpy(pathname, "./");

    //If a directory is not empty, then move the root directory to 
    //the front of the pathname    
    } else {

        //Using string copy and string concat again, 
        //we can move the root dir, to the front of the path
        //if the we already have a path string
        strcpy(cwd_PathBuffer, ".");
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);
    }

    //Free our temp path buffer and directory copy
    //and set both to NULL, and return the pathname
    free(dir_Copy);
    free(cwd_PathBuffer);
    dir_Copy = NULL;
    cwd_PathBuffer = NULL;
    return pathname;

}

//Function to set current working directory(Cwd)
int fs_setcwd(char * pathname){

    //Malloc a pointer to set the CWD to this one,
    //set_ToThisDir, then call parse_DirectoryPath
    //to parse the full pathname for this CWD
    fdDir * set_ToThisDir = get_dir_path(pathname);

    if(set_ToThisDir == NULL){

        printf("Failed to malloc set_ToThisDir\n");
        return -1;

    }

    //Show the previous CWD
    printf("The previous CWD was: %s\n", fs_CWD -> d_name);

    //Free the previous fs_CWD and set the pointer for fs_CWD
    //to this CWD, set_ToThisDir
    free(fs_CWD);
    fs_CWD = set_ToThisDir;

    //Show the new, current CWD
    printf("The current CWD is now: %s\n", fs_CWD -> d_name);
    return 0;

}

//Function to get the block count currenlty stored in our VCB
unsigned int getVCB_BlockCount(uint64_t bl_number)
{
    int result = bl_number / JCJC_VCB->blockSize;
    if (bl_number % JCJC_VCB->blockSize > 0)
    {
        result++;
    }
    return result;
}



//Function to get how many bytes are needed from our VCB
unsigned int getVCB_num_bytes(uint64_t block_count)
{
    int result = block_count * JCJC_VCB -> blockSize;
    return result;
}


// delete the file
int fs_delete(char* filename)
{
    char *pathExculdeLastSlash = malloc(strlen(filename) + 1);
    if (pathExculdeLastSlash != NULL)
    {
        strcpy(pathExculdeLastSlash, filename);
    }
    else
    {
        printf("[mfs.c -- fs_delete] malloc pathExculdeLastSlash failed\n");
        return -1;
    }

    char *getFileName = get_path_last_slash(pathExculdeLastSlash);

    // find the directory that is expected for holding that file
    fdDir *parent = parsePath(pathExculdeLastSlash);

    // find the file starting location to delete
    uint64_t start = -1;
    uint64_t size = -1;
    for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
    {
        // printf("parent->dirEntry[i].file_name = %s\n", parent->dirEntry[i].file_name);
        // printf("parent->dirEntry[i].fileType = %d\n", parent->dirEntry[i].fileType);
        if (parent->dirEntry[i].dirUsed == SPACE_IN_USED &&
            parent->dirEntry[i].fileType == FILE_TYPE &&
            strcmp(parent->dirEntry[i].file_name, getFileName) == 0)
        {
            start = parent->dirEntry[i].dir_Location;
            // printf("parent->dirEntry[i].dir_Location = %d\n", parent->dirEntry[i].dir_Location);
            size = parent->dirEntry[i].fileSize;
            parent->dirEntry[i].dirUsed = SPACE_IS_FREE;
            parent->dirEntryPosition--;
            LBAwrtie_func(parent, parent->d_reclen,
                             parent->directoryStartLocation);

            // read the data again if it is updating cwd
            if (fs_CWD != NULL && parent->directoryStartLocation == fs_CWD->directoryStartLocation)
            {
                memcpy(fs_CWD, parent, sizeof(fdDir));
            }
            break;
        }
    }

    // release the blocks occupied by the directory
    if (releaseFreespace(start, getVCB_BlockCount(size)) != 0)
    {
        printf("releaseFreespace() falied in fs_delete\n");
        return -1;
    }

    printf("%s : %s was removed\n", filename, getFileName);

    free(pathExculdeLastSlash);
    free(getFileName);
    pathExculdeLastSlash = NULL;
    getFileName = NULL;
    // fs_rmdir(filename);
    return 0;
}

