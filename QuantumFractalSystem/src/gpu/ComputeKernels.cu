// QuantumFractalSystem — CUDA compute kernels
// This file is only compiled when QFS_ENABLE_GPU=ON (requires CUDA toolkit).

#ifdef QFS_HAS_GPU

#include <cuda_runtime.h>
#include <cstdio>

// ── Element-wise tanh activation kernel ─────────────────────────────────────

__global__ void computeKernel(const float* input, float* output, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        output[idx] = tanhf(input[idx]);
    }
}

// ── Host-side dispatch ──────────────────────────────────────────────────────

extern "C" void gpu_compute(const float* h_input, float* h_output, int size) {
    float *d_input = nullptr, *d_output = nullptr;
    size_t bytes = static_cast<size_t>(size) * sizeof(float);

    cudaMalloc(&d_input, bytes);
    cudaMalloc(&d_output, bytes);
    cudaMemcpy(d_input, h_input, bytes, cudaMemcpyHostToDevice);

    int threads = 256;
    int blocks  = (size + threads - 1) / threads;
    computeKernel<<<blocks, threads>>>(d_input, d_output, size);

    cudaMemcpy(h_output, d_output, bytes, cudaMemcpyDeviceToHost);
    cudaFree(d_input);
    cudaFree(d_output);
}

#endif // QFS_HAS_GPU
