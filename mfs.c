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
#include "bitmap.c"

//Function to parse pathname of a directory from a specifc entry
fdDir * parse_DirectoryEntry(struct fs_diriteminfo * dir_entry){

    //Check if passed-in directory entry is     
    //of type Directory, if not, print error
    if (dir_entry -> fileType != FT_DIRECTORY){

        printf("Dir entry is not of type Directory\n");
        return NULL;
    }

    //Get the block count of this directory entry, and by 
    //using the sizeOf, malloc a temp buffer for entry
    unsigned int fd_DirBlockCount = getVCB_BlockCount(sizeof(fdDir));
    char * de_Buffer = malloc(fd_DirBlockCount * JCJC_VCB -> blockSize);

    if(de_Buffer == NULL){

        printf("Failed to malloc Directory Entry Buffer\n");
        return NULL;
    }

    //Malloc a directory pointer that will contain the relative 
    //pathname for the passed in directory entry
    fdDir * relative_Pathname = malloc(sizeof(fdDir));

    if(relative_Pathname == NULL){

        printf("Failed to malloc relative_Pathname\n");
        return NULL;

    }

    //Using LBAread, update the directory entry start location
    //and copy the path from de_Buffer to our relative pathname pointer
    LBAread(de_Buffer, fd_DirBlockCount, dir_entry -> entry_StartLocation);
    memcpy(relative_Pathname, de_Buffer, sizeof(fdDir));


    return relative_Pathname;
}

fdDir * parse_DirectoryPath(char * DE_pathname){

    //Malloc a pointer for the absolute path for 
    //a directory in current working directory (CWD)
    fdDir * absolute_DirPath = malloc(sizeof(fdDir));
    if(absolute_DirPath == NULL){

        printf("Failed to malloc absolute_DirPath\n");
        return -1;

    }

    //Copy the current fs_CWD of the file system
    memcpy(absolute_DirPath, fs_CWD, sizeof(fdDir));

    //Make a pure copy of the DE_pathname, so that the 
    //original pathname will not be modified
    char * pure_DirPathCopy = malloc(strlen(DE_pathname) + 1);
    if(pure_DirPathCopy == NULL){

        printf("Failed to malloc pure_DirPathCopy");
        return -1;

    }

    strcpy(pure_DirPathCopy, DE_pathname);

    //We will use the delimeter "/", to split the strings
    //and represent new directories with the delim as well
    char * path_Token = strtok(pure_DirPathCopy, Delim);

    while(path_Token != NULL){

        //Check if current path_Token is not root "." or not empty ""
        if(strcmp(path_Token, ".") != 0 || strcmp(path_Token, "") != 0){

                //Loop through the directory entries in dir_DE_count
                //and check if any of the dir entries matches with the 
                //string tokens from strtok()
               for(int i = 1; i < MAX_DE; i++){

                //We need to make sure that each dir entry is allocated 
                //in a used space in the bitmap, as well as of type directory
                //and the entry must have a matching name
                if(absolute_DirPath -> dir_DE_count[i].isFreeOrUsed == freeSpace_USED &&
                    absolute_DirPath -> dir_DE_count[i].fileType == FT_DIRECTORY &&
                    strcmp(absolute_DirPath -> dir_DE_count[i].d_name, path_Token) == 0){

                    //Free the pointer buffer and parse the directory entry's full
                    //pathname by calling the parse_DirectoryEntry function
                    free(absolute_DirPath);
                    absolute_DirPath = parse_DirectoryEntry(absolute_DirPath -> dir_DE_count + 1);
                    break;

                    }
                }

        } 
            //After parsing, set the path_Token to NULL with the delim
            //to indicate end of string/path
            path_Token = strtok(NULL, Delim);
         
        }
            return absolute_DirPath;

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
        fdDir * parentDir_ptr = parse_DirectoryEntry(dir_Copy -> dir_DE_count + 1);

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
    fdDir * set_ToThisDir = parse_DirectoryPath(pathname);

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

