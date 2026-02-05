#include <stdio.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct AudioData{
    unsigned char *data;
    char *fileName;
};

int main(){

    HANDLE myHandle;
    WIN32_FIND_DATA FindFileData;
    const char* directory="esc-50-audio/img/*.png";

    int x,y;
    struct AudioData audioData[2000];

    // Getting first file out of the loop
    myHandle=FindFirstFileA(directory,&FindFileData);
    printf("%s\n",FindFileData.cFileName);
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
    audioData[0].data = stbi_load(path, &x, &y, NULL, 1);
    // _strdup allocates new memory and copies the string so the struct keeps its own
    audioData[0].fileName = _strdup(path);

    int counter=1;
    while(FindNextFileA(myHandle,&FindFileData)&& counter<2000){
        printf("%s\n",FindFileData.cFileName);
        snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
        audioData[counter].data = stbi_load(path, &x, &y, NULL, 1);
        audioData[counter].fileName = _strdup(path);
        counter++;
    }
    // Closing windows handler
    FindClose(myHandle);


    // Freeing the memory
    for(int i=0;i<counter;i++){
        stbi_image_free(audioData[i].data);
        free(audioData[i].fileName);
    }
    return 0;
}