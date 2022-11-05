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
        printf("[fsDir.c --- fs_mkdir()] Doesn't have empty directory\n");
        return -1;
    }

    strcpy(directories[empty_dir_location].d_name, pathname);
    directories[empty_dir_location].isUsed = 1;
    directories[empty_dir_location].fileType = 0;
    directories[empty_dir_location].position_DE = 0;

    Directory_Entry * current_DE;
    current_DE = malloc(sizeof(Directory_Entry));

    strcpy(current_DE->file_name, '.');
    // dir_location doesn't finish setting
    current_DE->fileType = 0;
    current_DE->fileSize = sizeof(fdDir);
    strcpy(current_DE->filePath, absolutePath);
    current_DE->dirUsed = 1;
    current_DE->d_reclen = MAX_NUMBER_OF_CHILDREN;

    Directory_Entry * parent_DE;
    parent_DE = malloc(sizeof(Directory_Entry));
    strcpy(parent_DE->file_name, '..');
    parent_DE->fileType = 0;
    parent_DE->fileSize = sizeof(fdDir);
    strcpy(parent_DE->filePath, parent->filePath);
    parent_DE->dirUsed = 1;

    memcpy(directories[empty_dir_location].dirEntry[0], (char *)current_DE, 512);
    memcpy(directories[empty_dir_location].dirEntry[1], (char *)parent_DE, 512);

    int blockCount = (MAX_DE * sizeof(Directory_Entry)) / 512 + 1;
    // TODO
    // LBAwrite((char *)directories, blockCount, ); // last paremeter for directories location, doesn't finish


    free(current_DE);
    current_DE = NULL;
    free(parent_DE);
    parent_DE = NULL;

    return 1;
}

// return the exactly directory we find
// return NULL if not found anything
fdDir * fs_opendir(const char *name)
{
    if (name == NULL)
    {
        printf("[fsDir.c --- fs_opendir()] name is NULL\n");
        return NULL;
    }

    Directory_Entry * de;

    // may set up the condition number later just set 21 for now
    for (int i = 0; i < 21; i++)
    {
        de = (Directory_Entry*)directories[i].dirEntry[0];
        if (strcmp(de->file_name, name) == 0) // find the name in the directory
        {
            return &directories[i]; // return the exactly directory we found
        }
    }

    // TODO: if not found anything, this code will not print error message

    return NULL; // return NULL if not found anything
}



// TODO: finish later
int fs_rmdir(const char *pathname)
{
    if (strcmp(pathname, ".") == 0 || strcmp(pathname, "..") == 0 || strcmp(pathname,"/"))
    {
        printf("[fsDir.c --- fs_rmdir] Can not remove directory: %s\n", pathname);
        return -1;
    }

    if (strlen(pathname) == 0)
    {
        printf("[fsDir.c --- fs_rmdir] Can not remove directory since your enter pathname's length is 0\n");
        return -1;
    }

    char absolutePath[256];

    strcpy(absolutePath, cwd); // copy previous path into absolute path

    if (strlen(cwd) > 1) // already have directory inside of cwd
    {
        strcat(absolutePath, "/"); // adding slash
    }

    strcat(absolutePath, pathname);

    int remove_index = -1;

    Directory_Entry * de_remove;

    for (int i = 0; i < 21; i++)
    {
        de_remove = (Directory_Entry*)directories[i].dirEntry[0];
        if (strcmp(de_remove->filePath, absolutePath) == 0)
        {
            remove_index = i;
            break;
        }
    }

    if (remove_index == -1)
    {
        printf("[fsDir.c --- fs_rmdir] Can not find the file/directory: %s\n", pathname);
        return -1;
    }

    Directory_Entry * dir_need_to_be_removed;

    if (directories[remove_index].fileType == 1) // is file
    {
        
    }



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
    
    int cur_token_length = 0;
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
    // cur_token_length = 7
    // ppnlength = 22
    // ./csc210/csc222/csc510     -> 23
    // 
    int k = 0;

    //TODO might be we can just make it toegther using k<=cur_token_length
    while(k<cur_token_length){
        partent_path_name[ppnLength - k] = 0;
        k++;
    }

    //if parent path is not our root directory
    // we need to delete the last "/"
    if(strlen(partent_path_name) > 1){
        partent_path_name[ppnLength - cur_token_length] = 0;
    }

    save_ptr = partent_path_name;

    return save_ptr;
}

int fs_isFile(char * filename){
    //set file path name array size is 256
    char file_path_name[256];
    
    
    if(filename[0] == '/'){
        strcpy(file_path_name, filename);
    }
    
    if(filename[0] != '/'){
        strcpy(file_path_name, cwd);
        int fpnLength = strlen(file_path_name);
        
        //-------------------- not sure
        if(fpnLength > 1){
        strcat(file_path_name, "/");
        }
        strcat(file_path_name, filename);

    }

    Directory_Entry * de;
    
    //to find our directory
    int deIndex = getDirIndex(de, filename);

    //if check the filetype is not equal to 1 will return 0
    if(directories[deIndex].fileType != 1){
        return 0;
    }

    return 1;
    
}

// return 1 -> directory, 0 -> otherwise
// changing files' type
int fs_isDir(char * path)
{
    // check if path is is '.' or '..'
    // if it is, nothing need to check
    if(strcmp(path, ".") == 0 || strcmp(path, "..") == 0)
    {
        return 1; // return 1-> directory
    }

    char file_path_name[256];

    if (path[0] != '/') // something in cwd
    {
        strcpy(file_path_name, cwd);
        if (strlen(file_path_name) > 1)
        {
            // if file_path_name'e length is greater than 1, which means it is a directory
            // therefore, we need add '/' to file_path_name to make sure adding path correctly
            strcat(file_path_name, "/");
        }
        strcat(file_path_name, path); // finish copy all of things to get the correct path
    }

    // create a new directory entry to find the directory index in the directory array
    Directory_Entry * de;

    int deIndex = -1;

    for (int i = 0; i < 21; i++) // set the maximum number of directories for now
    {
        de = (Directory_Entry *)directories[i].dirEntry[0];
        // check if the directory entry is found
        if (strcmp(file_path_name, de->filePath) == 0)
        {
            deIndex = i;
            break;
        }
    }

    if (deIndex == -1)
    {
        printf("[fsDir.c --- fs_isDir()] Givn path is not found\n");
        return 0; // otherwise should return 0
    }

    return 1;  // return 1 if it is directory

}

int fs_closedir(fdDir *dirp)
{

 if (dirp == NULL)
 {
    printf("[ERROR] fsDir.c closedir\n");
    return -1;
 }
//  free(dirp);
//  dirp = NULL;
 return 0;
}

int fs_stat(const char *path, struct fs_stat *buf){
    Directory_Entry * de;

    int number_of_child = 8;
    for(int i=0; i < number_of_child; i++){
        
        de = (Directory_Entry *)directories[0].dirEntry[i];

        if(strcmp(de->file_name, path) == 0){

            buf->st_size = de->fileSize;
            buf->st_blksize = JCJC_VCB->blockSize;
            buf->st_blocks = sizeof(fdDir) / JCJC_VCB->blockSize;
            //TODO make the time next time
            // buf->st_accesstime
            // buf->st_mtime
            // buf->st_ctime
            return 1;
        }
    }

    return 0;
}

int fs_readdir(fdDir *dirp)
{
    Directory_Entry * de;
	Directory_Entry *current_DE;
    Directory_Entry *position_DE;
	if (dirp->position_DE >= number_of_child)
	{
		dirp->position_DE = 0; // reset to 0
		// return NULL if we finish looping through our directory
		return NULL;
	}
	while (dirp->position_DE < number_of_child)
	{
		current_DE = (Directory_Entry *)dirp->dirEntry[dirp->position_DE];
		if (strlen(current_DE->file_name) > 0)
		{
			fsDi.d_reclen = current_DE->d_reclen;
			fsDi.fileType = current_DE->fileType;
			strcpy(fsDi.d_name, current_DE->fileName);
			dirp->position_DE++;
			return (&fsDi);
		}
		dirp->position_DE++;
	}
	dirp->position_DE = 0;
	return NULL;
}