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
