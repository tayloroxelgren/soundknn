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

    myHandle=FindFirstFileA(directory,&FindFileData);
    printf("%s\n",FindFileData.cFileName);
    int counter=0;
    while(FindNextFileA(myHandle,&FindFileData)){
        printf("%s\n",FindFileData.cFileName);
        char path[MAX_PATH];
        snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
        audioData[counter].data = stbi_load(path, &x, &y, NULL, 1);
        // _strdup allocates new memory and copies the string so the struct keeps its own
        // persistent filename (path is a temporary buffer that would otherwise go out of scope)
        audioData[counter].fileName = _strdup(path);
        counter++;
    }

    // Loads data into 1D array 256x256 so 2nd row starts at 256
    
    if (!audioData[0].data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return 1;
    }
    if (!audioData[1].data) {
        printf("Failed to load image: %s\n", stbi_failure_reason());
        return 1;
    }

    for(int i=11766;i<12766;i++){
        if(audioData[0].data[i]!=0){
            printf("img1 info: %u",audioData[0].data[i]);
            printf(" name %s",audioData[0].fileName);
            printf("\n");
        }
    }

    for(int i=11766;i<12766;i++){
        if(audioData[1].data[i]!=0){
            printf("img2 info: %u",audioData[1].data[i]);
            printf(" name %s",audioData[1].fileName);
            printf("\n");
        }
    }
    return 0;
}