#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <omp.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

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

    // Leaving out sqrt to save compute as it doesn't change distance rankings
    // distance=sqrtf(distance);
    return distance;
}

void computeDistanceMatrix(AudioData *audioData, float **distanceArrays,int nfiles, int counter){
    // Computes distance matrix for all images
    int matrixSize=nfiles*nfiles;
    unsigned int computeCounter=0;
    for(int i = 0; i < counter; i++){
        for(int j = 0; j < counter; j++){
            float d = EuclideanDistance(&audioData[i], &audioData[j]);
            distanceArrays[i][j] = d;
            computeCounter++;
        }
        // printf("Way through matrix compute: %d\n",(computeCounter/matrixSize)*100);
        printf("\rWay through matrix compute: %.2f%%",(computeCounter/(float)matrixSize)*100);
        fflush(stdout);
    }
    printf("\n");
}

void computeDistanceMatrixOMP(AudioData *audioData, float **distanceArrays,int nfiles, int counter){
    // Computes distance matrix for all images
    int matrixSize=2000*2000;
    unsigned int computeCounter=0;
    // Uses openmp to use all threads for this
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < counter; i++){
        for(int j = 0; j < counter; j++){
            float d = EuclideanDistance(&audioData[i], &audioData[j]);
            distanceArrays[i][j] = d;
            #pragma omp atomic
            computeCounter++;
        }
        #pragma omp critical
        {
            printf("\rWay through matrix compute: %.2f%%",(computeCounter/(float)matrixSize)*100);
            fflush(stdout);
        }
    }
    printf("\n");
}


int getAmountOfFiles(){
    HANDLE myHandle;
    WIN32_FIND_DATA FindFileData;
    const char* directory="esc-50-audio/img/*.png";
    int x,y;
    int counter=0;

    // Getting first file out of the loop
    myHandle=FindFirstFileA(directory,&FindFileData);
    counter++;

    while(FindNextFileA(myHandle,&FindFileData)){
        counter++;
    }
    // Closing windows handler
    FindClose(myHandle);

    return counter;
}

int main(){

    HANDLE myHandle;
    WIN32_FIND_DATA FindFileData;
    const char* directory="esc-50-audio/img/*.png";

    int x,y;
    int nfiles=getAmountOfFiles();
    // Allocating on the heap to account for unknown number of images
    struct AudioData *audioData = malloc(sizeof(struct AudioData) * nfiles);

    // Getting first file out of the loop
    myHandle=FindFirstFileA(directory,&FindFileData);
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
    audioData[0].data = stbi_load(path, &x, &y, NULL, 1);
    // _strdup allocates new memory and copies the string so the struct keeps its own
    audioData[0].fileName = _strdup(path);

    int counter=1;
    // Gets all other files
    while(FindNextFileA(myHandle,&FindFileData)&& counter<nfiles){
        // printf("%s\n",FindFileData.cFileName);
        snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
        audioData[counter].data = stbi_load(path, &x, &y, NULL, 1);
        audioData[counter].fileName = _strdup(path);
        counter++;
    }
    // Closing windows handler
    FindClose(myHandle);

    float **distanceArrays = malloc(nfiles * sizeof(float*));

    for(int i=0;i<nfiles;i++){
        distanceArrays[i]=calloc(nfiles,sizeof(float));
    }

    time_t now = time(NULL);
    computeDistanceMatrixOMP(audioData, distanceArrays,nfiles, counter);
    printf("Time it took to compute matrix: %lld seconds\n",time(NULL)-now);

    // Searches for index of specific image
    char img[]="esc-50-audio/img/4-189838-A-22.png";
    int fileindex;
    for(int i=0;i<counter;i++){
        if(strcmp(img,audioData[i].fileName)==0){
            fileindex=i;
            break;
        }
    }

    int closest = -1;
    float best = INFINITY;

    for (int i = 0; i < counter; i++) {
        if (i != fileindex){
            // use matrix to find value
            float d = distanceArrays[fileindex][i];
            if (d < best) {
                best = d;
                closest = i;
            }
        }
    }
    
    printf("Original: %s\n",img);
    printf("Closest: %s",audioData[closest].fileName);

    // Freeing the memory of images
    for(int i=0;i<counter;i++){
        stbi_image_free(audioData[i].data);
        free(audioData[i].fileName);
    }

    // Freeing memeing of  distance arrays
    for(int i=0;i<nfiles;i++){
        free(distanceArrays[i]);
    }
    free(distanceArrays);

    free(audioData);
    return 0;
}