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

#define MAX_DE 50

//Variables if isFile or isDirectory
#define type_isDirectory 0
#define type_isFile 1


// This structure is returned by fs_readdir to provide the caller with information
// about each file as it iterates through a directory
struct fs_diriteminfo
	{
    unsigned short d_reclen;    /* length of this record */
    unsigned char fileType;    
    char d_name[256]; 			/* filename max filename is 255 characters */
	uint64_t	entry_StartLocation; 	//Variable for start location of either 
											// a file or directory
	};

// This is a private structure used only by fs_opendir, fs_readdir, and fs_closedir
// Think of this like a file descriptor but for a directory - one can only read
// from a directory.  This structure helps you (the file system) keep track of
// which directory entry you are currently processing so that everytime the caller
// calls the function readdir, you give the next entry in the directory
typedef struct
	{
	/*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
	unsigned short  d_reclen;		/*length of this record */
	unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
	uint64_t	directoryStartLocation;		/*Starting LBA of directory */
	uint64_t blockIndex;
	char dirEntry[10][512]; 		//dirEntry have 10 arrays with each array have length 512
	char d_name[128];
	int isUsed;									// 0 -> free, 1 -> used
	int fileType;								// 0 -> dir, 1 -> file
	struct fs_diriteminfo dir_DE_count[MAX_DE];


	// unsigned int create_date; 			//variable for file create date
	// unsigned int last_access_date; 			//variable for when you access/modify the file’s date
	// char  comment [300]; 			// comment for the file
	} fdDir;

// Key directory functions
int fs_mkdir(const char *pathname, mode_t mode);
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

//Struct for our VCB
typedef struct VCB{

	uint64_t numberOfBlocks;  			//integer variable for the number of blocks in VCB
	uint64_t blockSize;					//variable bytes for the size of each blocks in VCB (512 bytes) 

	unsigned int VCB_blockCount; 		//integer variable for starting position of free block space VCB (2560 bytes)
	uint64_t freeSpace_BlockCount; 		//integer variable for block count of free space in VCB
	uint64_t current_FreeBlockIndex;	//integer index variable for current free space starting position in VCB
	uint64_t first_freespace;
	
	uint64_t magicNumber;				//integer variable for magic number, useful when opening files based on their 
										//file Signature, and also for hex dumps 
					

	// int * free_block_location;			//the pointer to track our free space
	unsigned int free_block_count;		//the total numbers of the free blocks
	uint64_t location_RootDirectory;	//integer variable to hold root directory location in VCB

}volume_ControlBlock;


//VCB related functions
unsigned int getVCB_BlockCount(uint64_t);
fdDir * parse_DirectoryPath(char *);
fdDir * parse_DirectoryEntry(struct fs_diriteminfo *);


//Global variables for VCB, FreeSpace, and Directory
volume_ControlBlock * JCJC_VCB;
fdDir * rootDir_ptr;
fdDir * current_OpenedDir_ptr;
uint64_t current_OpenedDir_index;
int * freespace;
fdDir * directories;
fdDir * fs_CWD;


int init_VCB (uint64_t numberOfBlocks, uint64_t blockSize, __u_int blockCount_VCB);
void exitFileSystem ();
int init_freeSpace (volume_ControlBlock * JCJC_VCB);
int init__RootDir (volume_ControlBlock * JCJC_VCB);




typedef struct Directory_Entry{

	char file_name[256]; 				//character variable to store file name
	unsigned int dir_Location;			//integer variable to store file location
	size_t fileSize;					//variable for file size
	
	int fileType;						// 0->dir    1->file
	char filePath[256];				 	//file path
	int dirUsed;						//to check if any Dir is in use, 0 is free, 1 is using
	// unsigned directory_entry; 		// there is 60 bytes directory entry

	// time_t create_date; 				//variable for file create date
	// time_t last_access_date; 		//variable for when you access/modify the file’s date
	// char  comment [300]; 			// comment for the file

}Directory_Entry;



#endif

