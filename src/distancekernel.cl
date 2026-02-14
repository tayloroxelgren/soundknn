__kernel void EuclideanDistance(__global unsigned const char *imgs,
                                int offset1,
                                int imsize,
                                __global float *distance)
{

    int img1Offset = offset1;
    int imgsize     = imsize;

    int groupID = get_group_id(0);
    int img2Offset=imsize*groupID;

    int localID = get_local_id(0);
    int globalID = get_global_id(0);
    int groupSize = get_local_size(0);
    int totalSize = get_global_size(0);

    unsigned int sum = 0;


    // Subtraction
    for (int i = localID; i < imgsize; i += groupSize) {
        int d = (int)imgs[img1Offset + i] - (int)imgs[img2Offset + i];
        sum += (unsigned int)(d * d);
    }

    __local unsigned int partial[256];
    partial[localID] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (int stride = get_local_size(0)/2; stride > 0; stride >>= 1) {
        if (localID < stride) {
            partial[localID] += partial[localID + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (localID == 0) {
        distance[groupID] = (float)partial[0];
    }
}