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
    char * read_buffer = malloc(fdDir_block_count * JCJC->blockSize);

    if(read_buffer != NULL){
        LBAread(read_buffer, fdDir_block_count, entry->entry_StartLocation);
    }else{
        printf("malloc read_buffer failed!");
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




















































// //Function to get the current working directory(CWD)
// char * fs_getcwd(char * pathname, size_t size){

//     //Check if the string length of the CWD is less than 
//     //the size of the our pathname buffer
//     if(strlen(cwd) < size){

//         //Copy this specifc cwd to our pathname pointer
//         strncpy(pathname, cwd, size);

//     } else { //If the condition fails, print error message, and return NULL
        
//         printf("Length of CWD exceeds pathname buffer's size\n");
//         return NULL;
//     }

//     return pathname;

// }

// //Function to set current working directory(Cwd)
// int fs_setcwd(char * pathname){

//     //Create an array to hold our new CWD path
//     char go_ToThisDir[MAX_PATHSIZE];

//     //Check if the passed-in directory is the root directory
//     //if it is, then copy it into our new CWD path array
//     if(pathname[0] == Delim){

//         strcpy(go_ToThisDir, pathname);

//     } else {

//         /**************Start of Parsing Directory Path******************/

//         //Check if the pathname starts with "..", if so
//         //then copy this as the parent directory in our new CWD path
//         if (pathname[0] == "." && pathname[1] == "."){

//             char * parent_Dir_ptr = get_parent_path(cwd);

//             if(parent_Dir_ptr == NULL){

//                 printf("Failed to get parent directory's pathname\n");
//                 return -1;
//             }

//         strcpy(go_ToThisDir, parent_Dir_ptr);

//         //If the pathname starts with ".", then it is the root
//         //directory. If so, then copy this as root in our new CWD
//         } else if (pathname[0] == "."){

//             strcpy(go_ToThisDir, cwd);

//         } else {

//             //Otherwise, if the pathname is not root, then just
//             //copy the rest of the directory's path into our new CWD path
//             strcpy(go_ToThisDir, cwd);

//         //Checks if our CWD path array only has the root directory, 
//         //if it does not, then parse each directory by concatting with 
//         //our delim "/"
//         if(strlen(go_ToThisDir) != 1){

//             strcat(go_ToThisDir, Delim);

//         } if(pathname[0] == "." && pathname[1] == "."){

//                 //Check to see if the CWD exists by calling getcwd() function,
//                 //if not, then print error message 
//                 if(getcwd(pathname, cwd) == NULL){

//                     printf("Failed to get the CWD\n");
//                     return -1;

//                 }

//         }

//             //Parse together the path string into our new CWD path array
//             strcat(go_ToThisDir, pathname);

//         }

//     }

//     /*****************End of Parsing Directory Path********************/

//     //Init a temporary placeholder for a dir_entry
//    Directory_Entry * placeholder_DirEntry;

//     //Init a directory index that will search through our new CWD array
//    int direc_Index = getDirIndex(placeholder_DirEntry, go_ToThisDir);

//    //If the directory is empty, meaning the index is -1, 
//    //then print an error message 
//    if(direc_Index == -1){

//     printf("Failed: this directory is not found\n");
//     return -1;

//     //Check if directory entry is of type Directory,
//     //if not then print error
//    } else if (directories[direc_Index].fileType != 0){

//     printf("Current entry/item is not of type Directory\n");
//     return -1;

//    }

//     //Finally, copy this new CWD into the file system's cwd
//     strcpy(cwd, go_ToThisDir);

//     //Print the new CWD
//     printf("The current CWD is now: %s", cwd);

//     return 0;
// }

// // this function used to set up the directory entry's data
// // TODO: update setDirectoryEntry to each file later
// Directory_Entry * setDirectoryEntry(Directory_Entry * de, char fileName, size_t file_size,
//     int file_type, char path, int dirUsed)
// {
//     de = malloc(sizeof(Directory_Entry)); // mallocate the size of the directory entry
//     strcpy(de->file_name, fileName);
//     de->fileSize = file_size;
//     de->fileType = file_type;
//     strcpy(de->filePath, path);
//     de->dirUsed = dirUsed;

//     return de;
// }
