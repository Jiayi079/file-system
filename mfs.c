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

    //Create an array to hold our new CWD path
    char go_ToThisDir[MAX_PATHSIZE];

    //Check if the passed-in directory is the root directory
    //if it is, then copy it into our new CWD path array
    if(pathname[0] == Delim){

        strcpy(go_ToThisDir, pathname);

    } else {

        //Check if the pathname starts with "..", if so
        //then copy this as the parent directory in our new CWD path
        if (pathname[0] == "." && pathname[1] == "."){

            char * parent_Dir_ptr = get_parent_path(cwd);

            if(parent_Dir_ptr == NULL){

                printf("Failed to get parent directory's pathname\n");
                return -1;
            }

        strcpy(go_ToThisDir, parent_Dir_ptr);

        /**************Start of Parsing Directory Path******************/

        //If the pathname starts with ".", then it is the root
        //directory. If so, then copy this as root in our new CWD
        } else if (pathname[0] == "."){

            strcpy(go_ToThisDir, cwd);

        } else {

            //Otherwise, if the pathname is not root, then just
            //copy the rest of the directory's path into our new CWD path
            strcpy(go_ToThisDir, cwd);

        //Checks if our CWD path array only has the root directory, 
        //if it does not, then parse each directory by concatting with 
        //our delim "/"
        if(strlen(go_ToThisDir) != 1){

            strcat(go_ToThisDir, Delim);

        } if(pathname[0] == "." && pathname[1] == "."){

                //Check to see if the CWD exists by calling getcwd() function,
                //if not, then print error message 
                if(getcwd(pathname, cwd) == NULL){

                    printf("Failed to get the CWD\n");
                    return -1;

                }

        }

            //Parse together the path string into our new CWD path array
            strcat(go_ToThisDir, pathname);

        }

    }

    /*****************End of Parsing Directory Path********************/

    //Init a temporary placeholder for a dir_entry
   Directory_Entry * placeholder_DirEntry;

    //Init a directory index that will search through our new CWD array
   int direc_Index = getDirIndex(placeholder_DirEntry, go_ToThisDir);

   //If the directory is empty, meaning the index is -1, 
   //then print an error message 
   if(direc_Index == -1){

    printf("Failed: this directory is not found\n");
    return -1;

    //Check if directory entry is of type Directory,
    //if not then print error
   } else if (directories[direc_Index].fileType != 0){

    printf("Current entry/item is not of type Directory\n");
    return -1;

   }

    //Finally, copy this new CWD into the file system's cwd
    strcpy(cwd, go_ToThisDir);

    //Print the new CWD
    printf("The current CWD is now: %s", cwd);

    return 0;
}
