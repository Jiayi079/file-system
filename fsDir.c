#include "msf.h"
#include "fslow.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


// to get our parent path 
char * get_parent_path(char * pathname){
    char path_name[256];
    char * token;
    char * save_ptr;

    strcpy(path_name, pathname);
    
    //get our token
    token = strtok_r(path_name, "/", &save_ptr);
    //set our partent_path_name starting with "/"
    char partent_path_name[256] = "/";
    
    int cur_token_length;
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
    int k = 0;
    while(k<cur_token_length){
        partent_path_name[ppnLength - k] = 0;
        k++;
    }

    //if parent path is not our root directory
    // we need to delete the last "/"
    if(strlen(partent_path_name) > 1){
        partent_path_name[ppnLength - cur_token_length] = 0;
    }


    return save_ptr = partent_path_name;
    

    
}