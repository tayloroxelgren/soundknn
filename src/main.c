#include <stdio.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <omp.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


typedef struct AudioData{
    int index;
    char *fileName;
    int x;
    int y;
}AudioData;

void EuclideanDistance(unsigned char* imgs, int img1Offset,int img2Offset,int imsize,float *result){

    uint32_t distance=0;
    for(int i=0;i<imsize;i++){
        long d = imgs[img1Offset+i] - imgs[img2Offset+i];
        distance += (d * d);
    }

    // Leaving out sqrt to save compute as it doesn't change distance rankings
    // distance=sqrtf(distance);
    *result=(float)distance;
}

char* loadKernel(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open kernel file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* source = (char*)malloc(size + 1);
    fread(source, 1, size, file);
    source[size] = '\0';

    fclose(file);
    return source;
}

void computeDistanceMatrix(unsigned char* imgs,AudioData *audioData, float **distanceArrays,int nfiles, int counter){
    // Computes distance matrix for all images
    int matrixSize=nfiles*nfiles;
    unsigned int computeCounter=0;
    float distance;
    for(int i = 0; i < counter; i++){
        for(int j = 0; j < counter; j++){
            int imsize=audioData[i].x*audioData[i].y;
            EuclideanDistance(imgs, audioData[i].index*imsize,audioData[j].index*imsize,imsize,&distance);
            distanceArrays[i][j] = distance;
            computeCounter++;
        }
        printf("\rWay through matrix compute: %.2f%%",(computeCounter/(float)matrixSize)*100);
        fflush(stdout);
    }
    printf("\n");
}

void computeDistanceMatrixOMP(unsigned char* imgs,AudioData *audioData, float **distanceArrays,int nfiles, int counter){
    // Computes distance matrix for all images
    int matrixSize=nfiles*nfiles;
    unsigned int computeCounter=0;
    // Uses openmp to use all threads for this
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < counter; i++){
        for(int j = 0; j < counter; j++){
            float distance;
            int imsize=audioData[i].x*audioData[i].y;
            EuclideanDistance(imgs, audioData[i].index*imsize,audioData[j].index*imsize,imsize,&distance);
            distanceArrays[i][j] = distance;
            #pragma omp atomic
            computeCounter++;
        }
        if (omp_get_thread_num() == 0){
            printf("\rWay through matrix compute: %.2f%%",(computeCounter/(float)matrixSize)*100);
            fflush(stdout);
        }
    }
    printf("\n");
}


void computeDistanceOpenCL(unsigned char* imgs,AudioData *audioData, float **distanceArrays,int nfiles, int counter){
    // Computes distance matrix for all images
    cl_platform_id platform;
    cl_device_id device = 0;
    cl_context context = 0;
    cl_program program = 0;
    cl_command_queue commandQueue=0;
    cl_kernel kernel = 0;
    cl_int errNum;

    clGetPlatformIDs(1,&platform,NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    char name[256];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(name), name, NULL);
    printf("Device: %s\n", name);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    if(context == NULL){
        printf("Couldn't create context");
    }


    commandQueue=clCreateCommandQueue(context,device,0,&errNum);
    if(errNum!=CL_SUCCESS){
        printf("There was an error in the commmand queue");
    }

    char* kernelsourcecode= loadKernel("src/distancekernel.cl");
    program=clCreateProgramWithSource(context,1,(const char**)&kernelsourcecode ,NULL,NULL);
    clBuildProgram(program,1,&device,NULL,NULL,NULL);
    free(kernelsourcecode);
    kernel=clCreateKernel(program,(const char*)"EuclideanDistance",NULL);

    float distance[2000];
    cl_mem imgs_gpumem=clCreateBuffer(context,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(char)*nfiles*audioData[0].x*audioData[0].y,imgs,NULL);
    cl_mem distance_gpumem=clCreateBuffer(context,CL_MEM_WRITE_ONLY,sizeof(float)*2000,NULL,NULL);

    clSetKernelArg(kernel,0,sizeof(cl_mem),&imgs_gpumem);
    clSetKernelArg(kernel, 3, sizeof(cl_mem), &distance_gpumem);

    size_t globalWorkSize=256*2000;
    size_t localWorkSize  = 256;



    int matrixSize=nfiles*nfiles;
    unsigned int computeCounter=0;
    for(int i = 0; i < counter; i++){
        int imsize=audioData[i].x*audioData[i].y;
        int offset1=audioData[i].index*imsize;
        
        // Sets kernel args
        clSetKernelArg(kernel,1,sizeof(int),&offset1);
        clSetKernelArg(kernel,2,sizeof(int),&imsize);

        // Launches kernel
        clEnqueueNDRangeKernel(commandQueue,kernel,1,NULL,&globalWorkSize,&localWorkSize,0, NULL, NULL);
        // Read buffer back
        clEnqueueReadBuffer(commandQueue, distance_gpumem, CL_TRUE, 0, sizeof(float)*2000, distance, 0, NULL, NULL);
        for(int j=0;j<counter;j++){
            distanceArrays[i][j]=distance[j];
            computeCounter++;
        }
        printf("\rWay through matrix compute: %.2f%%",(computeCounter/(float)matrixSize)*100);
        fflush(stdout);
    }
    printf("\n");
    // releasing all objects 
    clReleaseMemObject(distance_gpumem);
    clReleaseMemObject(imgs_gpumem);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(commandQueue);
    clReleaseContext(context);
}


int getAmountOfFiles(){
    HANDLE myHandle;
    WIN32_FIND_DATA FindFileData;
    const char *directory="esc-50-audio/img/*.png";
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

    unsigned char *tmpData;

    tmpData = stbi_load(path, &x, &y, NULL, 1);
    unsigned char *imgData=malloc(sizeof(char)*x*y*nfiles);
    memcpy(imgData,tmpData,(size_t)x*y);
    stbi_image_free(tmpData);
    // _strdup allocates new memory and copies the string so the struct keeps its own
    audioData[0].fileName = _strdup(path);
    audioData[0].index=0;
    audioData[0].x=x;
    audioData[0].y=y;

    int counter=1;
    // Gets all other files
    while(FindNextFileA(myHandle,&FindFileData)&& counter<nfiles){
        snprintf(path, MAX_PATH, "esc-50-audio/img/%s", FindFileData.cFileName);
        tmpData=stbi_load(path, &x, &y, NULL, 1);
        memcpy(imgData+((size_t)counter*x*y),tmpData,(size_t)x*y);
        stbi_image_free(tmpData);
        audioData[counter].index = counter;
        audioData[counter].y = y;
        audioData[counter].x = x;
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
    // computeDistanceMatrix(imgData,audioData, distanceArrays,nfiles, counter);
    // computeDistanceMatrixOMP(imgData,audioData, distanceArrays,nfiles, counter);
    computeDistanceOpenCL(imgData,audioData, distanceArrays,nfiles, counter);
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
    free(imgData);

    // Freeing memeing of  distance arrays
    for(int i=0;i<nfiles;i++){
        free(distanceArrays[i]);
    }
    free(distanceArrays);

    free(audioData);
    return 0;
}