#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


typedef struct AudioData{
    unsigned char *data;
    char *fileName;
}AudioData;


float EuclideanDistance(AudioData *im1,AudioData *im2){
    // Using hard coded x,y for now
    int x,y;
    x=256;
    y=256;
    float distance=0;

    for(int i=0;i<(x*y);i++){
        float d = (float)im1->data[i] - (float)im2->data[i];
        distance += d * d;
    }


    return sqrtf(distance);
}

int main(){

    HANDLE myHandle;
    WIN32_FIND_DATA FindFileData;
    const char* directory="esc-50-audio/img/*.png";

    int x,y;
    struct AudioData audioData[2000];

    // Getting first file out of the loop
    myHandle=FindFirstFileA(directory,&FindFileData);
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
    audioData[0].data = stbi_load(path, &x, &y, NULL, 1);
    // _strdup allocates new memory and copies the string so the struct keeps its own
    audioData[0].fileName = _strdup(path);

    int counter=1;
    // Gets all other files
    while(FindNextFileA(myHandle,&FindFileData)&& counter<2000){
        // printf("%s\n",FindFileData.cFileName);
        snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
        audioData[counter].data = stbi_load(path, &x, &y, NULL, 1);
        audioData[counter].fileName = _strdup(path);
        counter++;
    }
    // Closing windows handler
    FindClose(myHandle);

    float *distanceArrays[2000];
    for(int i=0;i<2000;i++){
        distanceArrays[i]=calloc(2000,sizeof(float));
    }

    // Searches for index of specific image
    char img[]="esc-50-audio/img/1-32318-A-0.png";
    int fileindex;
    for(int i=0;i<counter;i++){
        if(strcmp(img,audioData[i].fileName)==0){
            fileindex=i;
            break;
        }
    }
    float distance=EuclideanDistance(&audioData[fileindex],&audioData[0]);
    int closest=fileindex;

    for(int i=0;i<counter;i++){
        if(strcmp(img,audioData[i].fileName)!=0){
            float newdistance=EuclideanDistance(&audioData[fileindex],&audioData[i]);
            if(newdistance<distance){
                closest=i;
                distance=newdistance;
            }
        }

    }
    printf("Closest : %s",audioData[closest].fileName);

    // Freeing the memory of images
    for(int i=0;i<counter;i++){
        stbi_image_free(audioData[i].data);
        free(audioData[i].fileName);
    }

    // Freeing memeing of  distance arrays
    for(int i=0;i<2000;i++){
        free(distanceArrays[i]);
    }
    return 0;
}