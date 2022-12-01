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
// #include "helperFunctions.c"

//#define MAX_ENTRIES_NUMBER 8
//#define MAX_NAME_LENGTH 256

// The mkdir() function creates a new, empty directory with a given name filename
// return 0  -> successfully
// return -1 -> failed
int fs_mkdir(const char *pathname, mode_t mode)
{
    // make a copy and use that for substring
    char *path_before_last_slash = malloc(strlen(pathname) + 1);
    if (path_before_last_slash != NULL)
    {
        strcpy(path_before_last_slash, pathname);
    }
    else
    {
        printf("[mfs.c -- fs_mkdir] malloc path_before_last_slash failed!\n");
        return -1;
    }

    // get the left patt as the new directory name by substringing
    char *new_dir_name = get_path_last_slash(path_before_last_slash);
    if (strcmp(new_dir_name, "") == 0)
    {
        printf("there no director name\n");
        return -1;
    }
    if (new_dir_name == NULL)
    {
        printf("get_path_last_slash failed!\n");
        return -1;
    }

    // get the directory pointer
    fdDir *parent_dir = parsePath(path_before_last_slash);
    if (parent_dir == NULL)
    {
        printf("'%s' is not exisited from cwd\n", path_before_last_slash);

        // avoid memory leak
        free(path_before_last_slash);
        free(new_dir_name);
        path_before_last_slash = NULL;
        new_dir_name = NULL;
        return -1;
    }

    // create direcotory
    // check if the dirEntryPosition is less than the maximum number of ammount entries -> 8
    if (parent_dir->dirEntryPosition < MAX_ENTRIES_NUMBER)
    {
        // can hold dirEntry
        // check if the space is free or not
        // check if the name already exists
        for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
        {
            if (parent_dir->dirEntry[i].dirUsed == SPACE_IN_USED &&
                strcmp(parent_dir->dirEntry[i].file_name, new_dir_name) == 0)
            {
                printf("[mfs.c -- mkdir] name already exsist in directory\n");

                // avoid memory leak
                free(path_before_last_slash);
                free(new_dir_name);
                free(parent_dir);
                path_before_last_slash = NULL;
                new_dir_name = NULL;
                parent_dir = NULL;
                return -1;
            }
        }

        // create the new directory
        // malloc() a new directory and clean it up
        fdDir *createdDir = malloc(sizeof(fdDir));
        if (createdDir != NULL)
        {
            memset(createdDir, 0, sizeof(fdDir));
        }
        else
        {
            printf("[mfs.c -- fs_mkdir] malloc createdDir failed\n");
            return -1;
        }

        // initialize the directory and allocate the space for it
        int freeSpaceLocation = allocateFreeSpace_Bitmap(getVCB_BlockCount(sizeof(fdDir)));
        if (freeSpaceLocation > 0)
        {
            createdDir->directoryStartLocation = freeSpaceLocation;
            createdDir->d_reclen = sizeof(fdDir);
            createdDir->dirEntryPosition = 2;
        }
        else
        {
            printf("[mfs.c -- mkdir] don't have enough free space\n");
            return -1;
        }

        // check if new_dir_name is too long
        if (strlen(new_dir_name) > (MAX_NAME_LENGTH - 1))
        {
            char *tempName = malloc(MAX_NAME_LENGTH);
            if (tempName == NULL)
            {
                printf("[mfs.c -- mkdir] malloc tempName failed\n");
                return -1;
            }
            else
            {
                // make sure it only contains one less than the max for null terminator
                strncpy(tempName, new_dir_name, MAX_NAME_LENGTH - 1);
                tempName[MAX_NAME_LENGTH - 1] = '\0';
                strcpy(createdDir->d_name, tempName);

                free(tempName);
                tempName = NULL;
            }
        }
        else
        {
            strcpy(createdDir->d_name, new_dir_name);
        }

        // initialize current directory entry .
        strcpy(createdDir->dirEntry[0].file_name, ".");
        createdDir->dirEntry[0].fileType = DIR_TYPE;
        createdDir->dirEntry[0].dirUsed = SPACE_IN_USED;
        createdDir->dirEntry[0].dir_Location = freeSpaceLocation;
        createdDir->dirEntry[0].d_reclen = sizeof(struct Directory_Entry);
        createdDir->dirEntry[0].fileSize = sizeof(fdDir);

        // initialize parent directory entry
        if (parent_dir->dirEntry == NULL)
        { // if parent is NULL, this is a root directory
            memcpy(createdDir->dirEntry + 1, createdDir->dirEntry, sizeof(struct Directory_Entry));
            // parent directory needs to be renamed to ".."
            strcpy(createdDir->dirEntry[1].file_name, "..");
        }
        if (parent_dir->dirEntry != NULL)
        {
            // if parent is not NULL, need to copy the parent into out entry
            memcpy(createdDir->dirEntry + 1, parent_dir->dirEntry, sizeof(struct fs_diriteminfo));
            // parent directory needs to be renamed to ".."
            strcpy(createdDir->dirEntry[1].file_name, "..");
        }

        // Since we cleaned newDir above and SPACE IS FREE = 0, this step can be skipped.
        // Instead, cycle through the remaining entries to designate them as free.
        // i = 2 because we set parent directory entry into 1, so it should be start at 2
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            createdDir->dirEntry[i].dirUsed = SPACE_IS_FREE;
        }

        // finding the available free space starting location
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            if (parent_dir->dirEntry[i].dirUsed == SPACE_IS_FREE)
            {
                // if dirEntry is free, put the data in
                parent_dir->dirEntryPosition++;
                parent_dir->dirEntry[i].d_reclen = sizeof(struct Directory_Entry);
                parent_dir->dirEntry[i].fileType = DIR_TYPE;
                parent_dir->dirEntry[i].dir_Location = createdDir->directoryStartLocation;
                //printf("dir_location in mkdir: %d\n", parent_dir->dirEntry[i].dir_Location);
                parent_dir->dirEntry[i].dirUsed = SPACE_IN_USED;
                parent_dir->dirEntry[i].fileSize = sizeof(fdDir);
                strcpy(parent_dir->dirEntry[i].file_name, createdDir->d_name);

                // write the two changed files back into the disk
                LBAwrtie_func(createdDir, createdDir->d_reclen,
                                 createdDir->directoryStartLocation);

                // read the data again if it is updating cwd
                if (fs_CWD != NULL && createdDir->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, createdDir, sizeof(fdDir));
                }

                LBAwrtie_func(parent_dir, parent_dir->d_reclen,
                                 parent_dir->directoryStartLocation);

                // read the data again if it is updating cwd
                if (fs_CWD != NULL && parent_dir->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, parent_dir, sizeof(fdDir));
                }

                printf("Creating directory %s success\n", parent_dir->dirEntry[i].file_name);

                break;
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

// pathname: test2.txt
int fs_mkFile(const char *pathname, mode_t mode)
{
    // make a copy and use that for substring
    char *path_before_last_slash = malloc(strlen(pathname) + 1);
    if (path_before_last_slash != NULL)
    {
        strcpy(path_before_last_slash, pathname);
    }
    else
    {
        printf("[mfs.c -- fs_mkdir] malloc path_before_last_slash failed!\n");
        return -1;
    }

    // get the left patt as the new directory name by substringing
    // new_file_name = test3.txt
    char *new_file_name = get_path_last_slash(path_before_last_slash);
    // printf("new_file_name: %s\n", new_file_name);
    if (strcmp(new_file_name, "") == 0)
    {
        printf("there no director name\n");
        return -1;
    }
    if (new_file_name == NULL)
    {
        printf("get_path_last_slash failed!\n");
        return -1;
    }

    // get the directory pointer
    fdDir *parent_dir = parsePath(path_before_last_slash);
    // printf("parent_dir: %s\n", parent_dir->d_name);
    if (parent_dir == NULL)
    {
        printf("'%s' is not exisited from cwd\n", path_before_last_slash);

        // avoid memory leak
        free(path_before_last_slash);
        free(new_file_name);
        path_before_last_slash = NULL;
        new_file_name = NULL;
        return -1;
    }

    // create direcotory
    // check if the dirEntryPosition is less than the maximum number of ammount entries -> 8
    if (parent_dir->dirEntryPosition < MAX_ENTRIES_NUMBER)
    {
        // can hold dirEntry
        // check if the space is free or not
        // check if the name already exists
        for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
        {
            if (parent_dir->dirEntry[i].dirUsed == SPACE_IN_USED &&
                strcmp(parent_dir->dirEntry[i].file_name, new_file_name) == 0)
            {
                printf("check name already exsist in directory, ");
                printf("you may want to copy file\n");

                // avoid memory leak
                free(path_before_last_slash);
                free(new_file_name);
                free(parent_dir);
                path_before_last_slash = NULL;
                new_file_name = NULL;
                parent_dir = NULL;
                return -1;
            }
        }

        // create the new file
        // malloc() a new file and clean it up
        fdDir *createdFile = malloc(sizeof(fdDir));
        if (createdFile != NULL)
        {
            memset(createdFile, 0, sizeof(fdDir));
        }
        else
        {
            printf("[mfs.c -- fs_mkdir] malloc createdDir failed\n");
            return -1;
        }

        // initialize the directory and allocate the space for it
        int freeSpaceLocation = allocateFreeSpace_Bitmap(getVCB_BlockCount(sizeof(fdDir)));
        if (freeSpaceLocation > 0)
        {
            createdFile->directoryStartLocation = freeSpaceLocation;
            createdFile->d_reclen = sizeof(fdDir);
            createdFile->dirEntryPosition = 2;
        }
        else
        {
            printf("[mfs.c -- mkdir] don't have enough free space\n");
            return -1;
        }

        // check if new_file_name is too long
        if (strlen(new_file_name) > (MAX_NAME_LENGTH - 1))
        {
            char *tempName = malloc(MAX_NAME_LENGTH);
            if (tempName == NULL)
            {
                printf("[mfs.c -- mkdir] malloc tempName failed\n");
                return -1;
            }
            else
            {
                // make sure it only contains one less than the max for null terminator
                strncpy(tempName, new_file_name, MAX_NAME_LENGTH - 1);
                tempName[MAX_NAME_LENGTH - 1] = '\0';
                strcpy(createdFile->d_name, tempName);

                free(tempName);
                tempName = NULL;
            }
        }
        else
        {
            strcpy(createdFile->d_name, new_file_name);
        }

        // initialize current directory entry .
        strcpy(createdFile->dirEntry[0].file_name, ".");
        createdFile->dirEntry[0].fileType = DIR_TYPE;
        createdFile->dirEntry[0].dirUsed = SPACE_IN_USED;
        createdFile->dirEntry[0].dir_Location = freeSpaceLocation;
        printf("freespaceLocation: %d\n", freeSpaceLocation);
        createdFile->dirEntry[0].d_reclen = sizeof(struct Directory_Entry);
        createdFile->dirEntry[0].fileSize = sizeof(fdDir);

        // initialize parent directory entry
        if (parent_dir->dirEntry == NULL)
        { // if parent is NULL, this is a root directory
            memcpy(createdFile->dirEntry + 1, createdFile->dirEntry, sizeof(struct Directory_Entry));
            // parent directory needs to be renamed to ".."
            strcpy(createdFile->dirEntry[1].file_name, "..");
        }
        if (parent_dir->dirEntry != NULL)
        {
            // if parent is not NULL, need to copy the parent into out entry
            memcpy(createdFile->dirEntry + 1, parent_dir->dirEntry, sizeof(struct Directory_Entry));
            // parent directory needs to be renamed to ".."
            strcpy(createdFile->dirEntry[1].file_name, "..");
        }

        // Since we cleaned newDir above and SPACE IS FREE = 0, this step can be skipped.
        // Instead, cycle through the remaining entries to designate them as free.
        // i = 2 because we set parent directory entry into 1, so it should be start at 2
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            createdFile->dirEntry[i].dirUsed = SPACE_IS_FREE;
        }

        // finding the available free space starting location
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            if (parent_dir->dirEntry[i].dirUsed == SPACE_IS_FREE)
            {
                // if dirEntry is free, put the data in
                parent_dir->dirEntryPosition++;
                parent_dir->dirEntry[i].d_reclen = sizeof(struct Directory_Entry);
                parent_dir->dirEntry[i].fileType = FILE_TYPE;
                parent_dir->dirEntry[i].dir_Location = createdFile->directoryStartLocation;
                parent_dir->dirEntry[i].dirUsed = SPACE_IN_USED;
                parent_dir->dirEntry[i].fileSize = sizeof(fdDir);
                strcpy(parent_dir->dirEntry[i].file_name, createdFile->d_name);

                // write the two changed files back into the disk
                LBAwrtie_func(createdFile, createdFile->d_reclen,
                                 createdFile->directoryStartLocation);

                // read the data again if it is updating cwd
                if (fs_CWD != NULL && createdFile->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, createdFile, sizeof(fdDir));
                }

                LBAwrtie_func(parent_dir, parent_dir->d_reclen,
                                 parent_dir->directoryStartLocation);

                // read the data again if it is updating cwd
                if (fs_CWD != NULL && parent_dir->directoryStartLocation == fs_CWD->directoryStartLocation)
                {
                    memcpy(fs_CWD, parent_dir, sizeof(fdDir));
                }

                break;
            }
        }
    }
    else
    {
        printf("Directory doesn't have enough space\n");
        free(path_before_last_slash);
        path_before_last_slash = NULL;
        free(new_file_name);
        new_file_name = NULL;
        free(parent_dir);
        parent_dir = NULL;
        return -1;
    }

    free(path_before_last_slash);
    path_before_last_slash = NULL;
    free(new_file_name);
    new_file_name = NULL;
    free(parent_dir);
    parent_dir = NULL;
    return 0;
}

int fs_rmdir(const char * pathname)
{
    // finding the directory by using pathname
    char *path = malloc(strlen(pathname) + 1);
    if (path != NULL)
    {
        strcpy(path, pathname);
    }
    else
    {
        printf("[mfs.c -- fs_rmdir] malloc path failed\n");
        return -1;
    }

    // get the directory full path
    fdDir * target = parsePath(path);

    // we'll never used path variable later
    // set allocate be free and NULL in case of error
    free(path);
    path = NULL;

    // check if the path we find is the root directory
    if (target->directoryStartLocation == JCJC_VCB->location_RootDirectory)
    {
        // if it is the root directory
        // we can not remove the root directory
        printf("root can't be removed\n");

        // allocate free in case of error
        free(target);
        target = NULL;
        return -1; // ending this functon since the pathname is the root directory
    }

    // get the parent's directory entry
    // by using to remove the directory in the parent
    fdDir *parent = parseEntry(target->dirEntry + 1);
    if (parent == NULL)
    {
        printf("[mfs.c -- fs_rmdir] parseEntry() on parent failed\n");
        return -1;
    }

    // we have to remove the directories both "." and ".."
    // since "." refers to this deleted file and ".." refers to its parent directory
    if (target->dirEntryPosition > 2)
    {
        // so we need to check if the target's directory entry count is greater than the 2
        // which is inlcude "." and ".."
        for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
        {
            // check if the directory entry is used
            // whcih means that we find the directory
            if (target->dirEntry[i].dirUsed == SPACE_IN_USED)
            {
                // since pathname and d_name will having slash to separate each other
                // we have to add 2 to malloc the size of this tempPath
                char *tempPath = malloc(strlen(pathname) + strlen(target->dirEntry[i].file_name) + 2);
                if (tempPath != NULL)
                {
                    // copy the pathname and the d_name to the tempPath
                    // also need to add slash to separate them
                    strcpy(tempPath, pathname);
                    strcat(tempPath, "/");
                    strcat(tempPath, target->dirEntry[i].file_name);
                }
                else
                {
                    // mallocate failed
                    printf("[mfs.c -- fs_rmdir]malloc tempPath failed\n");
                    return -1;
                }

                // check if tempPath present to dir
                if (fs_isDir(tempPath))
                {
                    // remove dir by using fs_rmdir
                    if (fs_rmdir(tempPath) != 0)
                    {
                        printf("fs_rmdir()");
                        return -1;
                    }
                }

                // check if tempPath present to file
                if (fs_isFile(tempPath))
                {
                    // remove file by using fs_delete
                    if (fs_delete(tempPath) != 0)
                    {
                        return -1;
                    }
                }
                else
                {
                    return -1;
                }
            }
        }
    }

    // check if the directory is going to be deleted, redirect cwd to the parent
    if (target->directoryStartLocation == fs_CWD->directoryStartLocation)
    {
        printf("redirecting to parent while cwd is being removed\n");
        fs_setcwd("..");
    }

    // after delte the file/path, also need to set the parent directory become free
    for (int i = 2; i < MAX_ENTRIES_NUMBER; i++)
    {
        // finding the correct directory
        if (parent->dirEntry[i].dirUsed == SPACE_IN_USED &&
            parent->dirEntry[i].fileType == DIR_TYPE &&
            strcmp(parent->dirEntry[i].file_name, target->d_name) == 0)
        {
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

    // releasing the directory's block-occupied space
    if (releaseFreespace(target->directoryStartLocation, getVCB_BlockCount(target->d_reclen)) != 0)
    {
        printf("releaseFreespace() falied in rmdir\n");
        return -1;
    }

    printf("%s : %s was removed\n", pathname, target->d_name);

    free(target);
    free(parent);
    target = NULL;
    parent = NULL;

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

//Function to write back data into our volume 
int LBAwrtie_func(void * fdDir, uint64_t length, uint64_t startingPosition)
{
    uint64_t fullBlockSize = getVCB_BlockCount(length) * JCJC_VCB->blockSize;

    //Malloc a temp_LBABuffer to copy data to 
    char *temp_LBABuffer = malloc(fullBlockSize);

    //If the temp temp_LBABuffer is not NULL, then set this
    //temp_LBABuffer into our VCB, otherwise, print error message
    if (temp_LBABuffer != NULL)
    {
        memset(temp_LBABuffer, 0, fullBlockSize);
    }
    else
    {
        printf("Failed to mallod temp_LBABuffer\n");
        return -1;
    }

    //Copy the data froom the buffer so that we can
    //write it into our volume using LBAwrite()
    memcpy(temp_LBABuffer, fdDir, length);
    LBAwrite(temp_LBABuffer, getVCB_BlockCount(length), startingPosition);

    //Free our temp buffer and set it to NULL
    free(temp_LBABuffer);
    temp_LBABuffer = NULL;
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
//Function to open a directory from a given pathname
fdDir *fs_opendir(const char *pathname)
{
    //Make a pure copy of the path name by mallocing
    char *pure_PathCopy = malloc(strlen(pathname) + 1);

    //Print error message if pure_PathCopy is NULL
    if (pure_PathCopy == NULL)
    {
        printf("[Failed to malloc a pure_PathCopy\n");
        return NULL;
    }

    //Copy the current path of the directory into our 
    //pure_PathCopy and then get the root directory
    strcpy(pure_PathCopy, pathname);
    rootDir = parsePath(pure_PathCopy);

    //Free our malloced pure_PathCopy and set to NULL
    openedDirEntryIndex = 0;

    free(pure_PathCopy);
    pure_PathCopy = NULL;
    return rootDir;
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
            return (dirp->dirEntry + check_de_index);
        }
        check_de_index++;
    }
    //If there are no more entries to index, then return NULL
    return NULL;
}

//Function to close the directory and free the memory
//we have allocated in the temp_LBABuffer
int fs_closedir(fdDir *dirp)
{
    free(dirp);
    dirp = NULL;
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


//Function to check if the passed-in pure_PathCopy
//is of type Directory 
int fs_isDir(char * pathname)
{

    for (int i = 0; i <= strlen(pathname); i++)
    {
        // printf("check before if statement\n");
        if (pathname[i] == '.')
        {
            // free(tempPtr);
            // tempPtr = NULL;
            return 0;
        }
    }

    // printf("pathname: %s\n", pathname);
    for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
    {
        if (fs_CWD->dirEntry[i].fileType == FILE_TYPE &&
            strcmp(fs_CWD->dirEntry[i].file_name, pathname) == 0)
        {
            // printf("filename find: %s\n", fs_CWD->dirEntry[i].file_name);
            // printf("file type: %d\n", fs_CWD->dirEntry[i].fileType);
            return 0;
        }
    }


    fdDir *cwd_Pointer = fs_CWD;
    // printf("the original fs cwd is: %X\n", fs_CWD);

    //Replace the cwd by our open_dir if that is opened
    int dir_open = 0;
    if (rootDir == NULL)
    {
        printf("[mfs.c -- fs_isDir] directories is NULL\n");
        return -1;
    }
    else
    {
        dir_open = 1;
        fs_CWD = rootDir;
    }

    //Use parsePath to check if given pathname is of DIR_TYPE
    fdDir *tempPtr = parsePath(pathname);
    int result = 0;
    if (tempPtr == NULL)
    {
        printf("[mfs.c -- fs_isDir] tempPtr is NULL\n");
        return -1;
    }
    else
    {
        result = 1;
    }

    // used to check if the pathname haven't store in the dirEntry
    for (int i = 0; i < MAX_ENTRIES_NUMBER; i++)
    {
        if (fs_CWD->dirEntry[i].fileType == DIR_TYPE &&
            strcmp(fs_CWD->dirEntry[i].file_name, pathname) == 0)
        {
            //Set the CWD pointer back to the current working directory
            fs_CWD = cwd_Pointer;

            //Free our temp CWD pointer
            free(tempPtr);
            tempPtr = NULL;
            return result;
        }
    }

    //Set the CWD pointer back to the current working directory
    fs_CWD = cwd_Pointer;

    //Free our temp CWD pointer
    free(tempPtr);
    tempPtr = NULL;
    return result;
}
    
//Function to get the current working directory(CWD)
char * fs_getcwd(char * pathname, size_t size){

    strncpy(pathname, "", size); //set initial pathname for directory blank/empty string

    //Malloc a temp_LBABuffer that will hold the CWD pure_PathCopy
    char * cwd_PathBuffer = malloc(size);
    if(cwd_PathBuffer == NULL){

        printf("Failed to malloc CWD Buffer\n");
        return NULL;

    }

    //Malloc a copy of the directory, so we can loop through it
    //when we need to parse the pure_PathCopy
    fdDir * dir_Copy = malloc(sizeof(fdDir));
    if(dir_Copy == NULL){

        printf("Failed to malloc Directory Copy\n");
        return NULL;

    }

    memcpy(dir_Copy, fs_CWD, sizeof(fdDir));

    //Using a while-loop, block_StartPos from the current directory entry and working 
    //backwards, go back up one directory at a time until root location is
    //reached. The result will be the full pure_PathCopy of the directory entry 
    while(dir_Copy -> directoryStartLocation != JCJC_VCB -> location_RootDirectory){

        //Using string copy and string concatenation, parse the pure_PathCopy
        //of the dir entry, and seperate each with a "/"
        strcpy(cwd_PathBuffer, "/");
        strcat(cwd_PathBuffer, dir_Copy -> d_name);
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);

        //Pointer for the parent directory 
        fdDir * parentDir_ptr = parseEntry(dir_Copy -> dirEntry + 1);

        //Free our temp_LBABuffer copy of the original directory
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
        //we can move the root dir, to the front of the pure_PathCopy
        //if the we already have a pure_PathCopy string
        strcpy(cwd_PathBuffer, ".");
        strcat(cwd_PathBuffer, pathname);
        strcpy(pathname, cwd_PathBuffer);
    }

    //Free our temp pure_PathCopy temp_LBABuffer and directory copy
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
    // printf("fs_setcwd get dir path: %s\n", pathname);
    fdDir *nextDir = parsePath(pathname);
    if (nextDir == NULL)
    {
        printf("Cannot get the file to set the cwd\n");
        return -1;
    }

    //Show the previous CWD
    // printf("previous fs_CWD: %s\n", fs_CWD->d_name);

    //Free the previous fs_CWD and set the pointer for fs_CWD
    //to this CWD, set_ToThisDir
    free(fs_CWD);
    fs_CWD = nextDir;

    //Show the new, current CWD
    // printf("current fs_CWD: %s\n", fs_CWD->d_name);
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

// replase the block int the free space bit map
int releaseFreespace(uint64_t start, uint64_t count)
{
    // handle error of invalid count, generally not goint to happen
    if (start < (JCJC_VCB->freeSpace_BlockCount + JCJC_VCB->VCB_blockCount) ||
        count < 1 ||
        start + count > JCJC_VCB->numberOfBlocks)
    {
        //printf("start: %d, count: %d\n", start, count);
        printf("JCJC_VCB->freeSpace_BlockCount : %d\n",JCJC_VCB->freeSpace_BlockCount);
        printf("JCJC_VCB->VCB_blockCount : %d\n",JCJC_VCB->VCB_blockCount);
        printf("JCJC_VCB->numberOfBlocks : %ld\n",JCJC_VCB->numberOfBlocks);
        printf("invalid arg in releaseFreespace\n");
        return -2;
    }

    for (uint64_t i = 0; i < count; i++)
    {
        // handle error when setBitFree get in errors
        if (setBitFree(start + i, freespace) != 0)
        {
            // this won't run if checkBit() works as expected
            printf("setBitFree() failed, bit at %ld", start + i);

            // we should mark those back in order to recover
            for (i--; i >= 0; i--)
            {
                setBitFree(start + i, freespace);
            }
            return -1;
        }
    }

    // simply compare if the freed block is before the freeblock index
    if (start < JCJC_VCB->current_FreeBlockIndex)
    {
        // set the new first free block index and update JCJC_VCB
        // printf("current free block index changes to %ld\n", start);
        JCJC_VCB->current_FreeBlockIndex = start;
        LBAwrtie_func(JCJC_VCB, sizeof(volume_ControlBlock), 0);
    }

    // next step: finding free space
    // do a round up since it needs to turn bits into bytes
    uint64_t bytes = convertBitToBytes();

    // starts right behind vcb
    LBAwrtie_func(freespace, bytes, JCJC_VCB->VCB_blockCount);

    // testing freespace on removing
    // printf("\nused block index: ");
    // for (int i = 0; i < JCJC_VCB->numberOfBlocks; i++)
    // {
    //     if (checkBit(i, freespace) == SPACE_IN_USED)
    //     {
    //         printf("%d ", i);
    //     }
    // }

    return 0;
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

// check the directory already contains the file name (argvec[1]) or not
// usually used in mv cmd, when user want to move a file to another directory
// we  have to check if the file name already exists in this new directory or not
// return 1 -> contains the file name
// return 0 -> doesn't contain the file name
int checkContainFile(char * filename)
{
    //We keep tempCWD for later use to copy back to the fs_CWD,
    // since we have to set the root directory as fs_CWD
    // printf("check checkContainFile filename: %s\n", filename);
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
        printf("[mfs.c -- checkContainFile] malloc pathExculdeLastSlash failed in checkContainFile()\n");
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