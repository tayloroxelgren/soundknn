__kernel void EuclideanDistance(__global unsigned const char *imgs,
                                __global int *offset1_offset2_imsize_gpumem,
                                __global float *distance)
{
    unsigned int sum = 0;

    int img1Offset = offset1_offset2_imsize_gpumem[0];
    int img2Offset = offset1_offset2_imsize_gpumem[1];
    int imsize     = offset1_offset2_imsize_gpumem[2];

    int lid = get_local_id(0);
    int gid = get_global_id(0);
    int groupSize = get_local_size(0);

    __local unsigned int partial[256];

    for (int i = lid; i < imsize; i += groupSize) {
        int d = (int)imgs[img1Offset + i] - (int)imgs[img2Offset + i];
        sum += (unsigned int)(d * d);
    }

    partial[lid] = sum;
    barrier(CLK_LOCAL_MEM_FENCE);

    for (int stride = groupSize/2; stride > 0; stride >>= 1) {
        if (lid < stride) {
            partial[lid] += partial[lid + stride];
        }
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (lid == 0) {
        distance[0] = (float)partial[0];
    }
}