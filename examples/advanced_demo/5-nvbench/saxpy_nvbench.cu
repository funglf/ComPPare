#define HAVE_NV_BENCH
#include <comppare/comppare.hpp>

#include <vector>

__global__ void saxpy(float a, float *x, float *y, float*y_out, int n) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < n) [[likely]]
    y_out[i] = a * x[i] + y[i];
}

void saxpy_gpu(const float a, const std::vector<float> x, const std::vector<float> y, std::vector<float> y_out) {
    int n = x.size();

    float *d_x, *d_y, *d_y_out;
    cudaMalloc(&d_x, n * sizeof(float));
    cudaMalloc(&d_y, n * sizeof(float));
    cudaMalloc(&d_y_out, n * sizeof(float));
    cudaMemcpy(d_x, x.data(), n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_y, y.data(), n * sizeof(float), cudaMemcpyHostToDevice);

    int blockSize = 256;
    int numBlocks = (n + blockSize - 1) / blockSize;

    GPU_HOTLOOPSTART;
    saxpy<<<numBlocks, blockSize>>>(a, d_x, d_y, d_y_out, n);
    GPU_HOTLOOPEND;

    y_out.resize(n);
    cudaMemcpy(y_out.data(), d_y_out, n * sizeof(float), cudaMemcpyDeviceToHost);

    cudaFree(d_x);
    cudaFree(d_y);
}

void saxpy_cpu(const float a, const std::vector<float> x, const std::vector<float> y, std::vector<float> y_out) {
    y_out.resize(y.size());

    HOTLOOPSTART;
    for (size_t i = 0; i < x.size(); i++)
        y_out[i] = a * x[i] + y[i];
    HOTLOOPEND;
}

int main(int argc, char **argv)
{
    float a = 1.1f;
    int n = 1<<10;

    std::vector<float> x(n, 2.2f);
    std::vector<float> y(n, 3.3f);

    comppare::InputContext<float, std::vector<float>, std::vector<float>>::OutputContext<std::vector<float>> cmp(a, x, y);

    cmp.add("saxpy gpu", saxpy_gpu).nvbench();
    cmp.add("saxpy cpu", saxpy_cpu).nvbench().set_is_cpu_only(true);

    cmp.run(argc, argv);
}