# User Guide ComPPare -- Validation & Benchmarking Framework <!-- omit from toc -->
- [1. Getting Started](#1-getting-started)
  - [1.1. Install](#11-install)
  - [1.2. Basic Usage](#12-basic-usage)
  - [1.3. Basic Working Principle](#13-basic-working-principle)
- [2. User API Documentation](#2-user-api-documentation)
  - [2.1. Macros](#21-macros)
  - [2.2. ComPPare `main()`](#22-comppare-main)
  - [2.3. Google Benchmark Plugin](#23-google-benchmark-plugin)
  - [2.4. `DoNotOptimize()`](#24-donotoptimize)
  
# 1. Getting Started
## 1.1. Install
### 1.1.1. Clone repository <!-- omit from toc -->
```bash
git clone git@github.com:funglf/ComPPare.git --recursive
```
#### If submodules like google benchmark/ nvbench is not needed: <!-- omit from toc -->
```bash
git clone git@github.com:funglf/ComPPare.git
```

### 1.1.2. (Optional) Build Google Benchmark <!-- omit from toc -->
See [Google Benchmark Instructions](https://github.com/google/benchmark/blob/b20cea674170b2ba45da0dfaf03953cdea473d0d/README.md) 

### 1.1.3. Include ComPPare  <!-- omit from toc -->
In your C++ code, simply include the comppare header file by:
```c
#include <comppare/comppare.hpp>
```

## 1.2. Basic Usage
There are a few rules in order to use comppare.


### 1.2.1. Adopt the required function signature <!-- omit from toc -->
*Function output must be `void`*
and consists of the input, then output types
```cpp
void impl(const Inputs&... in,     // read‑only inputs
        Outputs&...      out);     // outputs compared to reference
```

### 1.2.2. Add `HOTLOOP` Macros <!-- omit from toc -->
In order to benchmark specific regions of code, following Macros `HOTLOOPSTART`, `HOTLOOPEND` are needed. The region in between will be ran multiple times in order to get an accurate timing. The region first runs certain iterations of warmup before actually running the benchmark iterations.

#### How to use `HOTLOOPSTART`/`HOTLOOPEND` Macros <!-- omit from toc -->
```cpp
void impl(const Inputs&... in,
        Outputs&...      out);
{
    /* 
    setup or overhead you DO NOT want to benchmark 
    -- memory allocation, data transfer, etc.
    */

    HOTLOOPSTART; // Macro of start of Benchmarking Region of Interest

    // ... perform core computation here ...

    HOTLOOPEND;   // Macro of end of Benchmarking Region of Interest
}
```

#### Alternate Macro <!-- omit from toc -->
Alternatively, the macro `HOTLOOP()` can be used to wrap the whole region.
```cpp
void impl(const Inputs&... in,
        Outputs&...      out);
{
    HOTLOOP(
    // ... perform core computation here ...
    );
}
```

### 1.2.3. Setting up Comparison in `main()` <!-- omit from toc -->
In `main()`, you can setup the comparison, such as defining the reference function, initializing input data, naming of benchmark etc.

> ### The SAXPY example will be used throughout this section to demonstrate on usage <!-- omit from toc -->
>**SAXPY** stands for **Single-Precision A·X Plus Y**.  [(Also see examples/saxpy)](../examples/saxpy/README.md)
> 
> It stands for the following operation:
> 
>$$
>y_{\text{out}}[i] = a \cdot x[i] + y[i]
>$$


Based on [Section [Basic Usage]](#2-basic-usage), we can define a `saxpy` function as:
```cpp
void saxpy_cpu(/*Input types*/
               float a,
               const std::vector<float> &x,
               const std::vector<float> &y_in,
               /*Output types*/
               std::vector<float> &y_out)
{
    HOTLOOPSTART;
    for (size_t i = 0; i < x.size(); ++i)
        y_out[i] = a * x[i] + y_in[i];
    HOTLOOPEND;
}
```
---
#### Step 1: Initialize Input data <!-- omit from toc -->
```cpp
/* Intialize Input data */
float a = 1.1f
std::vector x(1000, 1.0); 
std::vector y_in(1000, 2.0);
```
---
#### Step 2: Create a Comparison Object <!-- omit from toc -->
This step defines the input and output types of the functions you are about to test.

For `saxpy_cpu`, the **input types** are:
```cpp
/* INPUTS: */
float,
std::vector<float>,
std::vector<float>
```

Let's define an alias here based on the Input Types:
```cpp
using CMP_INPUT = comppare::
    /*Define Input Pack Types same as the function*/
    InputContext<
                    float,              /*float a*/
                    std::vector<float>, /*std::vector<float> x*/
                    std::vector<float>  /*std::vector<float> y*/
                >::
```

---

As for the **output types**:
```cpp
/* OUTPUTS: */
std::vector<float>
```
Let's define another alias based on the Output Types:
```cpp
using CMP = CMP_INPUT::
    /*Define Output Pack types same as the function*/
    OutputContext<
                    std::vector<float>  /*std::vector<float> y_out*/
                 >;
```

---

Finally, we can create our comparison object with the alias `CMP` and initialize with the input data:
```cpp
CMP cmp(a,x,y_in); // Input data initialized above
```

--- 
**Or, define all at once without alias:**

Therefore, we can define the comparison object as:
```cpp
comppare:: /*namespace comppare*/
    /*Define Input Pack Types same as the function*/
    InputContext<
                    float,              /*float a*/
                    std::vector<float>, /*std::vector<float> x*/
                    std::vector<float>  /*std::vector<float> y*/
                >::
    /*Define Output Pack types same as the function*/
    OutputContext<
                    std::vector<float>  /*std::vector<float> y_out*/
                 >;
    
    cmp(a,x,y_in);   //cmp object -- a,x,y data
```

#### Step 3: Register/Add Functions into framework <!-- omit from toc -->
After creating the `cmp` object, we can add functions into it.

To Define the reference function:
```cpp
cmp.set_reference(/*Displayed Name After Benchmark*/"saxpy reference", /*Function*/saxpy_cpu);
```

To Add more fucntions:
```cpp
cmp.add("saxpy gpu", saxpy_gpu);
``` 

#### Step 4: Run the Benchmarks <!-- omit from toc -->
Command line arguments are the parameters for run():
```cpp
cmp.run(argc, argv);
```

#### Step 5: Results <!-- omit from toc -->
Example Output:
```bash
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
============ ComPPare Framework ============
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

Number of implementations:             3
Warmup iterations:                   100
Benchmark iterations:                100
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

Implementation                  Func µs             ROI µs            Ovhd µs         Max|err|[0]        Mean|err|[0]       Total|err|[0]
saxpy reference                    38.01               15.97               22.05            0.00e+00            0.00e+00            0.00e+00
saxpy gpu                       73151.41                1.07            73150.34            3.30e+06            1.66e+06            1.70e+09  <-- FAIL
```
In this case, `saxpy_gpu` failed.

### 1.2.4. Command Line Options -- Iterations <!-- omit from toc -->
There are 2 commmand line options to control the number of warmup and benchmark iterations: 

#### `--warmups` Warmup Iterations <!-- omit from toc -->
When used with a unsigned 64 bit integer, this sets the number of warmup iterations before running benchmark runs.

Example:
```bash
./saxpy --warmups 1000
```

#### `--iters` Benchmark Iterations  <!-- omit from toc -->
When used with a unsigned 64 bit integer, this sets the number of benchmark iterations

Example:
```bash
./saxpy --iters 1000
```

## 1.3. Basic Working Principle

### 1.3.1. `HOTLOOP` Macros <!-- omit from toc -->

HOTLOOP Macros essentially wrap your region of interest in a lambda before running the lambda across the warmup and benchmark iterations. 
The region of iterest is being timed across all the benchmark iterations to obtain an average runtime. 

In `comppare/comppare.hpp`, `HOTLOOPSTART` and `HOTLOOPEND` are defined as:
```cpp
#define HOTLOOPSTART \
    auto &&hotloop_body = [&]() { /* start of lambda */

#define HOTLOOPEND     \
    }; /* end lambda */ \
``` 

Therefore, any code in between the 2 macros are simply wrapped into the lambda `hotloop_body`:
```cpp
HOTLOOPSTART;
foo();
HOTLOOPEND;

/* is equivalent to */

auto &&hotloop_body = [&]() {
    foo();
};
```

After that, the lambda is being ran:
```cpp
/* Warm-up */                                                      
for (i = 0; i < warmup_iterations; ++i) 
    hotloop_body();                                                
                
/* Benchmark */
auto start = now();                        
for (i = 0; i < benchmark_iterations; ++i)  
    hotloop_body();                                                
auto end = now();                        
```

### 1.3.2. Output Comparison  <!-- omit from toc -->

Each implementation follows the same signature:
```cpp
void impl(const Inputs&... in,     // read‑only inputs
        Outputs&...      out);     // outputs compared to reference
```
**Input** – Passed in by the framework. All candidates get exactly the same data.

**Output** – The framework creates private output objects and passes into the implementation by reference. Therefore any change in the object will be stored in the framework.

After each implementation has finished running, the framework compares each output against the reference implementation, and prints out the results in terms of difference/error and whether it has failed.


# 2. User API Documentation
## 2.1. Macros

### 2.1.1 hotloop Macros <!-- omit from toc -->
#### `HOTLOOPSTART` & `HOTLOOPEND` <!-- omit from toc -->
Used to wrap around region of **CPU functions/operations** for framework to benchmark

example:
```cpp
impl(...)
{
    HOTLOOPSTART;
    cpu_func();
    a+b;
    HOTLOOPEND;
}
```
#### `HOTLOOP()` <!-- omit from toc -->
Alternative of [HOTLOOPSTART/END](#hotloopstart--hotloopend)

example:
```cpp
impl(...)
{
    HOTLOOP(
    cpu_func();
    a+b;
    );
}
```
#### `GPU_HOTLOOPSTART` & `GPU_HOTLOOPEND` <!-- omit from toc -->
Host Macro to wrap around region of **GPU host functions/operations** for framework to benchmark. Supports both CUDA and HIP, provided that the host function is compiled with the respected CUDA-compiler-wrapper `nvcc` and HIP-compiler-wrapper `hipcc`

> **Warning** Do Not use this within GPU kernels.

example:
```cpp
gpu_impl(...)
{
    GPU_HOTLOOPSTART;
    kernel<<<...>>>(...)
    cudaMemcpy(...);
    GPU_HOTLOOPEND;
}
```
#### `GPU_HOTLOOP()` <!-- omit from toc -->
Alternative of [GPU_HOTLOOPSTART/END](#gpu_hotloopstart--gpu_hotloopend)

example:
```cpp
gpu_impl(...)
{
    GPU_HOTLOOP(
    kernel<<<...>>>(...)
    cudaMemcpy(...);
    );
}
```



### 2.1.2 Manual timer Macros <!-- omit from toc -->
#### `MANUAL_TIMER_START` & `MANUAL_TIMER_END` <!-- omit from toc -->
Used to wrap around region of CPU functions/operations **within [HOTLOOP](#hotloop-macros)**

> These set of macros can **ONLY be used once** within the Hotloop

Example -- Only times `a+b`:
```cpp
impl(...)
{
    HOTLOOPSTART;
    cpu_func();
    MANUAL_TIMER_START
    a+b;
    MANUAL_TIMER_END
    HOTLOOPEND;
}
```

#### `GPU_MANUAL_TIMER_START` & `GPU_MANUAL_TIMER_END` <!-- omit from toc -->
Used to wrap around region of GPU host functions/operations **within [HOTLOOP](#hotloop-macros)**. Supports both CUDA and HIP, provided that the host function is compiled with the respected CUDA-compiler-wrapper `nvcc` and HIP-compiler-wrapper `hipcc`

> These set of macros can **ONLY be used once** within the Hotloop


Example -- Only times kernel:
```cpp
gpu_impl(...)
{
    GPU_HOTLOOPSTART;
    GPU_MANUAL_TIMER_START;
    kernel<<<...>>>(...)
    GPU_MANUAL_TIMER_END;
    cudaMemcpy(...);
    GPU_HOTLOOPEND;
}
```

### 2.1.3 Custom iteration timer Macro <!-- omit from toc -->

#### `SET_ITERATION_TIME(us)` <!-- omit from toc -->
This Macro takes in a **floating point number representing time of current iteration in $\mu s$**

Example on mixed timers:
```cpp
mixed_impl(...)
{
    cudaEvent_t gpu_start, gpu_end;
    cudaEventCreate(&gpu_start);              \
    cudaEventCreate(&gpu_end);

    HOTLOOPSTART;
    /* Timing CPU region of cpu_func() only*/
    auto cpu_start = chrono::steady_clock::now();
    cpu_func();
    auto cpu_end = chrono::steady_clock::now();
    a+b
    /* Microseconds taken by cpu_func() */
    float cpu_us = std::chrono::duration<float, std::micro>(end - start).count();

    /* Timing GPU region of kernel only */
    cudaEventRecord(gpu_start);
    kernel<<<...>>>(...)
    cudaEventRecord(gpu_end);
    cudaMemcpy(...);

    /* Microseconds taken by gpu kernel */
    float gpu_ms;
    cudaEventElapsedTime(&ms_manual, gpu_start, gpu_end);
    float gpu_us = gpu_ms * 1e3;

    /* set the total iteration time in us */
    float total_iteration_us = cpu_us + gpu_us;
    SET_ITERATION_TIME(total_iteration_us);
    HOTLOOPEND;
}
```

## 2.2. ComPPare `main()`

### 2.2.1. Creating the ComPPare object  <!-- omit from toc -->

Given your implementation signature:

```cpp
void impl(const Inputs&... in,
          Outputs&...      out);
```

Instantiate the comparison context by forwarding the input arguments into the nested `OutputContext` constructor:

```cpp
comppare::InputContext<Inputs...>
          ::OutputContext<Outputs...> 
            cmp(in...);
```

* **`Inputs...`**
  Variadic template parameters corresponding to the **types** of the input arguments of `impl` (e.g. `float`, `std::vector<int>`, etc.).
* **`Outputs...`**
  Variadic template parameters corresponding to the **types** of the output arguments of `impl`.

The order of `Inputs...` and `Outputs...` must match the order in the `impl` signature. After construction, `cmp` is ready to have implementations registered (via `set_reference` / `add`) and executed (via `run`).



### 2.2.2. Setting Implementations for Framework <!-- omit from toc -->

#### `set_reference` <!-- omit from toc -->

Registers the “reference” implementation and returns its corresponding `Impl` descriptor for further configuration -- eg attaching to Plugins like Google Benchmark.


##### Context <!-- omit from toc -->

Member of

```cpp
comppare::InputContext<Inputs...>::OutputContext<Outputs...>
```

##### Signature <!-- omit from toc -->

```cpp
comppare::Impl& set_reference(
    std::string display_name,
    std::function<void(const Inputs&... , Outputs&...)> f
);
```

##### Parameters <!-- omit from toc -->

| Name           | Type                                                  | Description                                                                            |
| -------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `display_name` | `std::string`                                         | Human-readable label for this reference implementation. Used in output report. |
| `f`            | `std::function<void(const Inputs&... , Outputs&...)>` | Function matching the signature: `void(const Inputs&... in, Outputs&... out)` |


##### Returns <!-- omit from toc -->

* **`comppare::Impl&`**
    
    Reference to the internal `Impl` object representing the just-registered implementation. Used mainly for attaching to plugins like Google Benchmark. 

    Recommended to discard return value.
  



##### Example <!-- omit from toc -->

```cpp
void ref_impl(const Inputs&..., Outputs...){};
auto cmp = comppare::InputContext<Inputs...>::OutputContext<Outputs...>();

cmp.set_reference(/*display name*/"reference implementation", /*function*/ ref_impl);
```

---
#### `add` <!-- omit from toc -->

Registers additional implementation which will be compared against reference and returns its corresponding `Impl` descriptor for further configuration -- eg attaching to Plugins like Google Benchmark.



##### Context <!-- omit from toc -->

Member of

```cpp
comppare::InputContext<Inputs...>::OutputContext<Outputs...>
```

##### Signature <!-- omit from toc -->

```cpp
comppare::Impl& add(
    std::string display_name,
    std::function<void(const Inputs&... , Outputs&...)> f
);
```

##### Parameters <!-- omit from toc -->

| Name           | Type                                                  | Description                                                                            |
| -------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `display_name` | `std::string`                                         | Human-readable label for this reference implementation. Used in output report. |
| `f`            | `std::function<void(const Inputs&... , Outputs&...)>` | Function matching the signature: `void(const Inputs&... in, Outputs&... out)` |

<a id="comppare_main_add_returnimpl"></a>
##### Returns <!-- omit from toc -->
* **`comppare::Impl&`**
    
    Reference to the internal `Impl` object representing the just-registered implementation. Used mainly for attaching to plugins like Google Benchmark. 

    Recommended to discard return value.
  



##### Example <!-- omit from toc -->

```cpp
void ref_impl(const Inputs&..., Outputs...){};
auto cmp = comppare::InputContext<Inputs...>::OutputContext<Outputs...>();

cmp.set_reference(/*display name*/"reference implementation", /*function*/ ref_impl);
cmp.add(/*display name*/"Optimized memcpy", /*function*/ fast_memcpy_impl);
```

### 2.2.3. Running Framework <!-- omit from toc -->
#### `run` <!-- omit from toc -->
Runs all the added implementations into the comppare framework. 


##### Context <!-- omit from toc -->

Member of

```cpp
comppare::InputContext<Inputs...>::OutputContext<Outputs...>
```

##### Signature <!-- omit from toc -->

```cpp
void run(int argc, char** argv);
```

##### Parameters <!-- omit from toc -->
| Name           | Type                                                  | Description                                                                            |
| -------------- | ----------------------------------------------------- | -------------------------------------------------------------------------------------- |
| `argc` | `int`                                         | Number of Command Line Arguments |
| `argv`            | `char**` | Command Line Argument Vector |

##### Example <!-- omit from toc -->
```cpp
int main(int argc, char **argv)
{
    /* Create cmp object */
    cmp.run(argc, argv);
}
```

### 2.2.4. Summary of `main()` <!-- omit from toc -->
```cpp
void reference_impl(const Inputs&... in,
                    Outputs&...      out);

void optimized_impl(const Inputs&... in,
                    Outputs&...      out);

int main(int argc, char** argv)
{
    comppare::InputContext<Inputs...>
              ::OutputContext<Outputs...> 
                cmp(in...);
    
    cmp.set_reference("Reference", reference_impl);

    cmp.add("Optimized", optimized_impl);

    cmp.run(argc, argv);
}
```
For more concrete example, please see [examples](../examples/README.md).

## 2.3. Google Benchmark Plugin

#### `google_benchmark()` <!-- omit from toc -->

Attaches the Google Benchmark plugin when calling [`set_reference`](#set_reference) or [`add`](#add), enabling google benchmark to the current implementation.

---

##### Context <!-- omit from toc -->

Member of an internal struct

```cpp
comppare::Impl
```

##### Signature <!-- omit from toc -->

```cpp
benchmark::internal::Benchmark* google_benchmark();
```

##### Returns <!-- omit from toc -->

* **`benchmark::internal::Benchmark*`**
  Pointer to the underlying Google Benchmark `Benchmark` instance.
  Use this to chain additional benchmark configuration calls (e.g. `->Arg()`, `->Threads()`, `->Unit()`, etc.).

##### Detailed Description <!-- omit from toc -->

After registering an implementation via `set_reference` or `add`, both functions return a reference to an internal struct `comppare::Impl` [(see here)](#comppare_main_add_returnimpl). `google_benchmark()` attaches the plugin to the current implementation. The returned pointer allows you to further customize the benchmark before execution.

##### Example <!-- omit from toc -->
```cpp
cmp.set_reference("Reference", reference_impl)
   .google_benchmark();

cmp.add("Optimized", optimized_impl)
   .google_benchmark();

cmp.run(argc, argv);
```

### Manual Timing with Google Benchmark <!-- omit from toc -->
Enable manual timing for any registered implementation by appending `->UseManualTime()` to the Benchmark* returned from google_benchmark(). This instructs Google Benchmark to measure only the intervals you explicitly mark inside your implementation, with [Manual Timer Macros](#212-manual-timer-macros) or [SetIterationTime() macro](#set_iteration_timeus).

`UseManualTime()` is a Google Benchmark API call that switches the benchmark into Manual Timing mode. [(See Google Benchmark's Documentation)](https://google.github.io/benchmark/user_guide.html#manual-timing)

```cpp
impl_manualtimer_macro(...)
{
    HOTLOOPSTART;
    ...
    MANUAL_TIMER_START;
    ...
    MANUAL_TIMER_END;
    ...
    HOTLOOPEND;
}

impl_setitertime_macro(...)
{
    HOTLOOPSTART;
    ...
    double elapsed_us;
    SetIterationTime(elapsed_us)
    ...
    HOTLOOPEND;
}

int main()
{
    cmp.set_reference("Manual Timer Macro", impl_manualtimer_macro)
    .google_benchmark() 
        ->UseManualTime();

    cmp.add("SetIterationTime Macro", impl_setitertime_macro)
    .google_benchmark() 
        ->UseManualTime();
}
```

## 2.4. `DoNotOptimize()`
For a deep dive into the working principle of `DoNotOptimize()` please visit [examples/advanced_demo/DoNotOptimize](../examples/advanced_demo/4-DoNotOptimize/README.md)

### Disclaimer  <!-- omit from toc -->
I, LF Fung, am not the author of `DoNotOptimize()`. The implementation of `comppare::DoNotOptimize()` is a verbatim of Google Benchmark's `benchmark::DoNotOptimize()`.

### References: <!-- omit from toc -->
1. [CppCon 2015: Chandler Carruth "Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!"](https://www.youtube.com/watch?v=nXaxk27zwlk&t=3319s) <br>
2. [Google Benchmark Github Repository](https://github.com/google/benchmark) <br>


### 2.4.1. Problem with Compiler Optimsation <!-- omit from toc -->
Compiler optimization can sometimes remove operations and variables completely.

For instance in the following function:

```cpp
void SAXPY(const float a, const float* x, const float* y)
{
    float yout;
    for (int i = 0; i < N; ++i)
    {
        yout = a * x[i] + y[i];
    }
}
```

When compiling at high optimization, the compiler realizes `yout` is just a temporary that’s never used elsewhere. As a result, `yout` is optimized out, and thus the whole saxpy operation would be optimized out.

#### SAXPY() in Assembly <!-- omit from toc -->
When SAXPY() is compiled in AArch64 with Optimisation `-O3`
```asm
__Z5SAXPYfPKfS0_:                       ; @_Z5SAXPYfPKfS0_
	.cfi_startproc
; %bb.0:
	ret
	.cfi_endproc
                                        ; -- End function
```
The function body is practically empty: a single `ret` which is “return from subroutine” [Arm A64 Instruction Set: **RET**](https://developer.arm.com/documentation/dui0802/b/A64-General-Instructions/RET). In simple terms, it just returns — nothing happens.

### 2.4.2. Solution -- Google Benchmark's DoNotOptimize()  <!-- omit from toc -->
Optimization is important to understand the performance of particular operations in production builds. This creates the conflicting ideas of `optimize` but `do not optimize away`. This was solved by Google in their [benchmark](https://github.com/google/benchmark) -- a microbenchmarking library. Google Benchmark provides `benchmark::DoNotOptimize()` to prevent variables from being optimized away.

With the same SAXPY function, we simply add DoNotOptimize() around the temporary variable `yout`
```cpp
void SAXPY_DONOTOPTIMIZE(const float a, const float* x, const float* y)
{
    float yout;
    for (int i = 0; i < N; ++i)
    {
        yout = a * x[i] + y[i];
        DoNotOptimize(yout);
    }
}
```
This `DoNotOptimize` call tells the compiler not to eliminate the temporary variable, so the operation itself won’t be optimized away.

#### SAXPY_DONOTOPTIMIZE() in Assembly <!-- omit from toc -->
When SAXPY_DONOTOPTIMIZE() is compiled in AArch64 with Optimisation `-O3`:
<!-- <details> -->
<pre><code class="language-asm">
<!-- <span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">1</span>       .section        __TEXT,__text,regular,pure_instructions
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">2</span>       .build_version macos, 14, 0     sdk_version 14, 4
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">3</span>       .globl  __Z19SAXPY_DONOTOPTIMIZEfPKfS0_ ; -- Begin function _Z19SAXPY_DONOTOPTIMIZEfPKfS0_
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">4</span>       .p2align        2 --><span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">5</span> __Z19SAXPY_DONOTOPTIMIZEfPKfS0_:        ; @_Z19SAXPY_DONOTOPTIMIZEfPKfS0_
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">6</span>       .cfi_startproc
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">7</span> ; %bb.0:
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">8</span>       sub     sp, sp, #16
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">9</span>       .cfi_def_cfa_offset 16
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">10</span>      mov     w8, #16960
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">11</span>      movk    w8, #15, lsl #16
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">12</span>      add     x9, sp, #4
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">13</span>      add     x10, sp, #8
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">14</span> LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">15</span>      ldr     s1, [x0], #4
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">16</span>      ldr     s2, [x1], #4
<span style="display:inline-block;width:100%;white-space:pre;background-color:rgba(0, 255, 4, 0.49);border-radius:2px;"><span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">17</span>      fmadd   s1, s0, s1, s2</span>
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">18</span>      str     s1, [sp, #4]
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">19</span>      str     x9, [sp, #8]
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">20</span>      ; InlineAsm Start
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">21</span>      ; InlineAsm End
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">22</span>      subs    x8, x8, #1
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">23</span>      b.ne    LBB0_1
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">24</span> ; %bb.2:
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">25</span>      add     sp, sp, #16
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">26</span>      ret
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">27</span>      .cfi_endproc
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">28</span>                                         ; -- End function
</code></pre>
<!-- </details> -->
<br>

Further inspection reveals the Fused-Multiply-Add instruction -- indicating that SAXPY operation was not optimized away.
<pre><code class="language-asm">
<span style="opacity:0.5; font-size:smaller; display:inline-block; width:3em; text-align:right;">17</span>      fmadd   s0, s0, s1, s2
</code></pre>


[Reference to Arm A64 Instruction Set: **FMADD**](https://developer.arm.com/documentation/ddi0602/2025-06/SIMD-FP-Instructions/FMADD--Floating-point-fused-multiply-add--scalar--)

### 2.4.3. `comppare::DoNotOptimize()` <!-- omit from toc -->
Provided the usefulness of Google Benchmark's `benchmark::DoNotOptimize()`, comppare includes a verbatim of `benchmark::DoNotOptimize()`. 

Example:
```cpp
impl(...)
{
    ...
    comppare::DoNotOptimize(temporary_variable);
    ...
}

```