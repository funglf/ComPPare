# Advanced Demo 3 -- **Manual Timing** <!-- omit from toc -->

- [1. Introduction](#1-introduction)
- [2. Quick Start](#2-quick-start)
  - [2.1. Build](#21-build)
  - [2.2. Run](#22-run)
- [3. What are we benchmarking](#3-what-are-we-benchmarking)
  - [3.1. Necessity of Manual timing](#31-necessity-of-manual-timing)
- [4. How to Manual Time](#4-how-to-manual-time)
- [5. More control over Manual timing](#5-more-control-over-manual-timing)
- [6. Results](#6-results)


## 1. Introduction
Manually time sections within a hotloop. Needed when for instance some setup overhead is required within the ROI itself.

## 2. Quick Start
### 2.1. Build
```bash 
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make 
```

### 2.2. Run 
```bash
./manual_timing_demo --warmups 100000 --iters 100000
```

## 3. What are we benchmarking
We want to evaluate the performance of a simple function that operates on a `std::vector<int>`:
```cpp
static void f(std::vector<int> &v)
{
    v.clear();      // clear vector
    v.resize(N);    // resize vector
    random_fill(v); // Randomly fill 
    square(v);      // square each element
}
```
This function performs four distinct operations on the vector. To better understand their relative cost, we run four separate benchmarks:

1. `All` – the full function, including `clear`, `resize`, `random_fill`, and `square`.

2. `Resize` – only the resize operation.

3. `Random Fill` – only the random filling of values.

4. `Clear + Square` – the combination of clearing the vector and squaring its elements.


### 3.1. Necessity of Manual timing
In many benchmarks, the function under test cannot be isolated cleanly.
Often there is unavoidable setup or cleanup work inside the benchmark loop that you don’t actually want to measure.

Suppose we benchmark `square` operating on contents of a vector.
Since the operation is in-place, after one iteration the vector contents have already changed.
Before the next iteration, we must reset the vector back to its original random values.

```cpp
static void bench_square(std::vector<int> &v)
{
    HOTLOOPSTART;

    // Reset step (needed before every iteration)
    random_fill(v);

    // We only want to time the squaring step
    MANUAL_TIMER_START;
    square(v);
    MANUAL_TIMER_END;

    HOTLOOPEND;
}
```



## 4. How to Manual Time
By default, benchmarks measure the total runtime of the hotloop section.
If you only want to measure part of the function (e.g. resize), you can use the provided manual timing macros:

- `MANUAL_TIMER_START` – start measuring time.

- `MANUAL_TIMER_END` – stop measuring time.

The comppare framework will deal with it as usual after adding these macros.

```cpp
static void bench_resize(std::vector<int> &v)
{
    HOTLOOPSTART;   // Marks start of benchmark loop
    v.clear();

    // Only measure the resize step
    MANUAL_TIMER_START;
    v.resize(N);
    MANUAL_TIMER_END;

    random_fill(v);

    square(v);
    HOTLOOPEND;     // Marks end of benchmark loop
}
```

## 5. More control over Manual timing 
Sometimes when we want finer control — for example, benchmarking multiple sections in the same iteration.
In that case, you can directly measure durations yourself and report them using the `SET_ITERATION_TIME()` macro.

> Note: you cannot use `MANUAL_TIMER_START/END` macros multiple times within same loop.

```cpp
static void bench_clear_square(std::vector<int> &v)
{
    HOTLOOPSTART;

    // only time clear
    auto clear_start = std::chrono::steady_clock::now();
    v.clear();
    auto clear_end = std::chrono::steady_clock::now();

    v.resize(N);

    random_fill(v);

    // only time square
    auto square_start = std::chrono::steady_clock::now();
    square(v);
    auto square_end = std::chrono::steady_clock::now();

    auto iter_duration = (square_end - square_start) + (clear_end - clear_start);

    // Set iteration time
    SET_ITERATION_TIME(iter_duration);
    
    HOTLOOPEND;
}
```

## 6. Results 
`All` is roughly equivalent to `Resize` + `Random Fill` + `Clear + Square` which is what we would expect!


```bash
*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=
============ ComPPare Framework ============
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

Number of implementations:             4
Warmup iterations:                100000
Benchmark iterations:             100000
=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*

Implementation              ROI µs/Iter            Func µs            Ovhd µs         Max|err|[0]        Mean|err|[0]       Total|err|[0]
All                                 4.06           405684.08                0.21                   0                   0                   0                   
Resize                              0.05           394796.96           389873.24                   0                   0                   0                   
Random Fill                         3.93           403436.54            10430.84                   0                   0                   0                   
Clear + Square                      0.07           400374.12           393651.59                   0                   0                   0     
```