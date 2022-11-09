/**************************************************************
* Class:  CSC-415-01 Fall 2022
* Names:Jiaming Zhao, Jiayi Gu, Carmelo De Guzman, Congcheng Zeng
* Student IDs:921891383, 920024739, 918749005, 918327792
* GitHub Name:akizhao614, Jiayi079, carmelodz, Congchengz
* Group Name:JCJC
* Project: Basic File System
*
* File: fsDir.h
*
* Description: This file contains functions on getting a
*               specifc directory entry index as well as
*               getting the index of an unused directory
*
*
*
**************************************************************/

#include "mfs.h"
#include "mfs.h"

int getDirIndex(Directory_Entry * Dir, char *path){
    int i;
    for(i=0; i<10; i++){
        Dir = (Directory_Entry *)directories[i].dirEntry[0];
        //if we compare the path if the same, that means we found it 
        //and return the the index we find
        if(strcmp(Dir->filePath, path) != 0){
            //if not the same, we will return -1 and means works
            return -1;
        }else{
            //it will break the for loop
            break;
        }
    }

    //return the index value from our for loop
     return i;
}


int getNotInUseDir(){
    int  i = -1;
    while(i < 10){
        if(directories[i].dirUsed == 0){
            //if we find the unuse dir, if will return the index value
            return i;
        }
        i++;
    }

    //if not find will reutn -1
    return -1;
}