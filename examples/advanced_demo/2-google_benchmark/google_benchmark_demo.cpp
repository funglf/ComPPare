#include <vector>
#include <algorithm>
#include <execution>

#include <comppare/comppare.hpp>

void square_single_thread(const std::vector<int> &vec, std::vector<int> &out)
{
    out.resize(vec.size());
    HOTLOOPSTART;
    for (size_t i = 0; i < vec.size(); ++i)
    {
        out[i] = vec[i] * vec[i];
    }
    HOTLOOPEND;
}

void square_omp(const std::vector<int> &vec, std::vector<int> &out)
{
    out.resize(vec.size());
    HOTLOOPSTART;
#pragma omp parallel for
    for (size_t i = 0; i < vec.size(); ++i)
    {
        out[i] = vec[i] * vec[i];
    }
    HOTLOOPEND;
}

void square_transform(const std::vector<int> &vec, std::vector<int> &out)
{
    out.resize(vec.size());
    HOTLOOPSTART;
    std::transform(std::execution::par_unseq,
                   vec.begin(), vec.end(),
                   out.begin(),
                   [](int x)
                   { return x * x; });
    HOTLOOPEND;
}

int main(int argc, char **argv)
{
    std::vector<int> input(1000000);
    std::iota(input.begin(), input.end(), 1);

    // Create an instance of the ComPPare framework
    auto compare = comppare::make_comppare<std::vector<int>>(input);

    // Register the implementations 
    // Add .google_benchmark() to enable Google Benchmark for the particular implementation
    compare.set_reference("Single Thread", square_single_thread).google_benchmark()
                                                                 ->Unit(benchmark::kMillisecond);
                                                                 
    compare.add("OpenMP", square_omp).google_benchmark()
                                      ->Unit(benchmark::kMillisecond);

    compare.add("C++ Parallel", square_transform); // No google benchmark for this one

    compare.run(argc, argv);
}