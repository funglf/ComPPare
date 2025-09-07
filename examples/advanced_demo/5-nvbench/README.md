# Advanced Demo 5 -- **nvbench** <!-- omit from toc -->

- [1. Introduction](#1-introduction)
- [2. Quick Start](#2-quick-start)
  - [2.1. Build](#21-build)
    - [2.1.1. Optionally define CUDA arch](#211-optionally-define-cuda-arch)
  - [2.2. Run](#22-run)
- [3. nvbench on top of ComPPare](#3-nvbench-on-top-of-comppare)
  - [3.1. How to use](#31-how-to-use)
- [4. nvbench CLI](#4-nvbench-cli)


## 1. Introduction
nvbench is a provided external plugin. It is like google benchmark, but more geared towards nvidia gpu kernels.

## 2. Quick Start
### 2.1. Build
```bash 
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make 
```

#### 2.1.1. Optionally define CUDA arch
```bash
cmake .. -DCUDA_ARCH=80
```

### 2.2. Run 
```bash
./saxpy_nvbench
```

## 3. nvbench on top of ComPPare
### 3.1. How to use
Use `GPU_HOTLOOPSTART/END` to mark the region you want to benchmark. 

```cpp
void saxpy_gpu(const float a, const std::vector<float> x, const std::vector<float> y, std::vector<float> y_out) {
    /* Setup */

    GPU_HOTLOOPSTART;
    saxpy<<<numBlocks, blockSize>>>(a, d_x, d_y, d_y_out, n);
    GPU_HOTLOOPEND;

    /* Cleanup */
}
```

To attach to nvbench, simply add `.nvbench()`:
```cpp
cmp.add("saxpy gpu", saxpy_gpu).nvbench();
```

You can also add nvbench options like:
```cpp
cmp.add("saxpy cpu", saxpy_cpu).nvbench().set_is_cpu_only(true);
```

## 4. nvbench CLI
nvbench has CLI arguments. To use them, add prefix `--nvbench` to the argument. For instance to set the throttle threshold of the gpu:
```bash
./saxpy_nvbench --nvbench="--throttle-threshold 20" --iters 1000 --warmups 1000
```

Only `--throttle-threshold 20` will be passed into nvbench:
```bash
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
============= nvbench =============
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

nvbench cmdline arguments:
    [0] "./saxpy_nvbench"
    [1] "--throttle-threshold"
    [2] "20"
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
```

