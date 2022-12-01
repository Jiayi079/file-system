/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: mfs.h
*
* Description: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/
#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "b_io.h"
#include "fsLow.h"
#include "helperFunctions.h"

#include <dirent.h>
#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

// used with bitmap
#define SPACE_IS_FREE 0
#define SPACE_IN_USED 1

#define DIR_TYPE 0
#define FILE_TYPE 1

#define MAX_ENTRIES_NUMBER 20
#define MAX_NAME_LENGTH 256

// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
{
	unsigned short d_reclen; 			/* length of this record */
	unsigned char fileType;
	unsigned char isFreeOrUsed;		  	// determine this entry is free or used
	uint64_t directoryStartLocation;  	// LBA of the entry, either a file or directory
	uint64_t fileSize;				  	// the exact size of the file occupies
	char d_name[MAX_NAME_LENGTH]; 		/* filename max filename is 255 characters */
};

// used to hold a data file information about menu
struct Directory_Entry
{ 
	unsigned short d_reclen; 			/* length of this record */
	unsigned char fileType;				// 0->dir 1->file 
	unsigned char dirUsed;		 		//to check if any Dir is in use, 0 is free, 1 is using 
	uint64_t dir_Location;  			//integer variable to store file location 
	size_t fileSize;				  	//variable for file size
	char file_name[MAX_NAME_LENGTH]; 	//character variable to store file name 

   	time_t create_time; 				//variable for file create time 
 	time_t last_access_time; 			//variable for when you access the file’s time
	time_t modified_time; 				//variable for when you modify the file's time
 	// char comment [300]; 				// comment for the file 

}Directory_Entry; 

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short d_reclen;		 		/*length of this record */
	uint64_t directoryStartLocation; 		/*Starting LBA of directory */
	unsigned short dirEntryPosition;	 	/*which directory entry position, like file pos */
	char d_name[MAX_NAME_LENGTH];	 		// name of this directory
	struct Directory_Entry dirEntry[MAX_ENTRIES_NUMBER];
} fdDir;

//Struct for our VCB
typedef struct VCB{
	uint64_t magicNumber;				// integer variable for magic number, useful 
										// when opening files based on their 
										// file Signature, and also for hex dumps 
	uint64_t blockSize;					// variable bytes for the size of each 
										// blocks in VCB (512 bytes) 
	uint64_t numberOfBlocks;	  		// integer variable for the number of blocks in VCB
	uint VCB_blockCount;			  	//integer variable for starting position of free 
										// block space VCB (2560 bytes)
	uint freeSpace_BlockCount;	  		// used for check when it is not the first run
	uint64_t current_FreeBlockIndex; 	// used for check when it is not the first run
	uint64_t location_RootDirectory;	// can be calculated by adding the other two counts
} volume_ControlBlock;

//VCB related functions
unsigned int getVCB_BlockCount(uint64_t bl_number);

//DIR relate functions
fdDir * parseEntry(struct Directory_Entry * entry);
char * get_path_last_slash(char * path);
fdDir * parsePath(char * name);
int checkContainFile(char * filename);

int LBAwrtie_func(void * fdDir, uint64_t length, uint64_t startingPosition);

//Global variables for VCB, FreeSpace, and Directory
volume_ControlBlock * JCJC_VCB;
fdDir * rootDir; // used for keep opened directory
int * freespace;
uint64_t openedDirEntryIndex;
fdDir *fs_CWD;


int releaseFreespace(uint64_t start, uint64_t count);

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
int fs_mkFile(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);

// Directory iteration functions
fdDir * fs_opendir(const char *pathname);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);

// Misc directory functions
char * fs_getcwd(char *pathname, size_t size);
int fs_setcwd(char *pathname);   //linux chdir
int fs_isFile(char * filename);	//return 1 if file, 0 otherwise
int fs_isDir(char * pathname);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file

// This is the strucutre that is filled in from a call to fs_stat
struct fs_stat
	{
	off_t     st_size;    		/* total size, in bytes */
	blksize_t st_blksize; 		/* blocksize for file system I/O */
	blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
	time_t    st_accesstime;   	/* time of last access */
	time_t    st_modtime;   	/* time of last modification */
	time_t    st_createtime;   	/* time of last status change */
	
	/* add additional attributes here for your file system */
	};

int fs_stat(const char *path, struct fs_stat *buf);

#endif
