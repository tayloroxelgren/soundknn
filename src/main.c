#include <stdio.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct AudioData{
    unsigned char *data;
    char *fileName;
};

int main(){
    int x,y;
    // Loads data into 1D array 256x256 so 2nd row starts at 256
    struct AudioData audioData[2];
    audioData[0].data= stbi_load("esc-50-audio/img/1-137-A-32.png", &x, &y, NULL, 1);
    audioData[1].data = stbi_load("esc-50-audio/img/1-977-A-39.png", &x, &y, NULL, 1);

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
            printf("\n");
        }
    }

    for(int i=11766;i<12766;i++){
        if(audioData[1].data[i]!=0){
            printf("img2 info: %u",audioData[1].data[i]);
            printf("\n");
        }
    }
    return 0;
}