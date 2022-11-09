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
#include "bitmap.c"
#include "fsDir.c"

//Function to get the current working directory(CWD)
char * fs_getcwd(char * pathname, size_t size){

    //Check if the string length of the CWD is less than 
    //the size of the directory's pathname
    if(strlen(cwd) < size){

        //Copy this specifc cwd to our pathname pointer
        strcpy(pathname, cwd);

    } else { //If the condition fails, print error message, and return NULL
        
        printf("Length of cwd exceeds pathname's size\n");
        return NULL;
    }

    return pathname;

}

//Function to set current working directory(Cwd)
int fs_setcwd(char * pathname){

    char go_ToThisDir[MAX_PATHSIZE];

    if(pathname[0] == Delim){

        strcpy(go_ToThisDir, pathname);

    } else {

        if (pathname[0] == "." && pathname[1] == "."){

            char * parent_Dir_ptr = get_parent_path(cwd);
            if(parent_Dir_ptr == NULL){

                printf("Failed to get parent directory's pathname\n");
                return -1;
            }

        strcpy(go_ToThisDir, parent_Dir_ptr);

        } else if (pathname[0] == "."){

            strcpy(go_ToThisDir, cwd);

        } else {

            strcpy(go_ToThisDir, cwd);

        if(strlen(go_ToThisDir) != 1){

            strcat(go_ToThisDir, Delim);

        } if(pathname[0] == "." && pathname[1] == "."){

                if(getcwd(pathname, cwd) == NULL){

                    printf("Failed to get the CWD\n");
                    return -1;

                }

        }

            strcat(go_ToThisDir, pathname);

        }

    }

   Directory_Entry * placeholder_DirEntry;

   int direc_Index = getDirIndex(placeholder_DirEntry, go_ToThisDir);
   if(direc_Index == -1){

    printf("Failed: this directory is not found\n");
    return -1;

   } else if (directories[direc_Index].fileType != 0){

    printf("Current entry/item is not of type Directory\n");
    return -1;

   }

    strcpy(cwd, go_ToThisDir);
    return 0;

}
