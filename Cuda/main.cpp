#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH 640
#define HEIGHT 480

#define BACTERIA_COLOR 0xFFFFFF // white
#define INACTIVE_BACTERIA_COLOR 0x000000 // black

#define MAX_SPEED 5
#define ACTIVATION_THRESHOLD 0.5

// CUDA kernel to update the color of each pixel
__global__ void updatePixels(int *pixels, float *velocities, float *activations) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x < WIDTH && y < HEIGHT) {
        // Get the index of the current pixel
        int idx = y * WIDTH + x;
        
        // Update the velocity of the bacteria at this pixel
        velocities[idx*2] += (float)rand()/(float)RAND_MAX * 2 - 1; // Random x-velocity
        velocities[idx*2+1] += (float)rand()/(float)RAND_MAX * 2 - 1; // Random y-velocity
        velocities[idx*2] = max(-MAX_SPEED, min(MAX_SPEED, velocities[idx*2])); // Clamp x-velocity
        velocities[idx*2+1] = max(-MAX_SPEED, min(MAX_SPEED, velocities[idx*2+1])); // Clamp y-velocity
        
        // Update the activation of the bacteria at this pixel
        activations[idx] += (float)rand()/(float)RAND_MAX * 0.1 - 0.05; // Random activation change
        activations[idx] = max(0.0, min(1.0, activations[idx])); // Clamp activation
        
        // Update the color of the pixel based on the activation of the bacteria
        if (activations[idx] >= ACTIVATION_THRESHOLD) {
            pixels[idx] = BACTERIA_COLOR;
        } else {
            pixels[idx] = INACTIVE_BACTERIA_COLOR;
        }
        
        // Move the bacteria based on its velocity
        int new_x = x + (int)velocities[idx*2];
        int new_y = y + (int)velocities[idx*2+1];
        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT) {
            // Swap the bacteria to the new position
            int new_idx = new_y * WIDTH + new_x;
            int tmp_pixel = pixels[idx];
            float tmp_velocity_x = velocities[idx*2];
            float tmp_velocity_y = velocities[idx*2+1];
            float tmp_activation = activations[idx];
            pixels[idx] = pixels[new_idx];
            pixels[new_idx] = tmp_pixel;
            velocities[idx*2] = velocities[new_idx*2];
            velocities[idx*2+1] = velocities[new_idx*2+1];
            velocities[new_idx*2] = tmp_velocity_x;
            velocities[new_idx*2+1]= tmp_velocity_y;
            activations[idx] = activations[new_idx]s;
            activations[new_idx] = tmp_activation;
        }
    }
}

int main() {
    int *pixels;
    float *velocities, *activations;
    cudaMalloc(&pixels, WIDTH * HEIGHT * sizeof(int));
    cudaMalloc(&velocities, WIDTH * HEIGHT * 2 * sizeof(float));
    cudaMalloc(&activations, WIDTH * HEIGHT * sizeof(float));
    srand(time(NULL));
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        pixels[i] = INACTIVE_BACTERIA_COLOR;
        velocities[i*2] = (float)rand()/(float)RAND_MAX * 2 - 1; // Random x-velocity between -1 and 1
        velocities[i*2+1] = (float)rand()/(float)RAND_MAX * 2 - 1; // Random y-velocity between -1 and 1
        activations[i] = (float)rand()/(float)RAND_MAX; // Random activation between 0 and 1
    }
    dim3 threadsPerBlock(16, 16);
    dim3 numBlocks((WIDTH + threadsPerBlock.x - 1) / threadsPerBlock.x,
                   (HEIGHT + threadsPerBlock.y - 1) / threadsPerBlock.y);
    for (int i = 0; i < 100; i++) {
        updatePixels<<<numBlocks, threadsPerBlock>>>(pixels, velocities, activations);
        cudaDeviceSynchronize();

    }
    cudaFree(pixels);
    cudaFree(velocities);
    cudaFree(activations);
    return 0;
}