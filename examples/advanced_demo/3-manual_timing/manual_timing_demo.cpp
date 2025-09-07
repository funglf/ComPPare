#include <vector>
#include <random>
#include <chrono>

#include <comppare/comppare.hpp>

#define N 1024

/*
Helper Functions
*/
static void random_fill(std::vector<int> &v)
{
    std::mt19937 gen(123);
    std::uniform_real_distribution<float> dist(0.0f, 1e3f);
    for (size_t i = 0; i < v.size(); ++i)
        v[i] = static_cast<int>(dist(gen));
}

static void square(std::vector<int> &v)
{
    for (size_t i = 0; i < v.size(); ++i)
        v[i] *= v[i];
}

/*
Benchmarking Functions
*/
static void bench_all(std::vector<int> &v)
{
    HOTLOOPSTART;
    v.clear();

    v.resize(N);

    random_fill(v);

    square(v);
    HOTLOOPEND;
}

static void bench_resize(std::vector<int> &v)
{
    HOTLOOPSTART;
    v.clear();

    MANUAL_TIMER_START;
    v.resize(N);
    MANUAL_TIMER_END;

    random_fill(v);

    square(v);
    HOTLOOPEND;
}

static void bench_randomfill(std::vector<int> &v)
{
    HOTLOOPSTART;
    v.clear();

    v.resize(N);

    MANUAL_TIMER_START;
    random_fill(v);
    MANUAL_TIMER_END;

    square(v);
    HOTLOOPEND;
}

static void bench_clear_square(std::vector<int> &v)
{
    HOTLOOPSTART;
    auto clear_start = std::chrono::steady_clock::now();
    v.clear();
    auto clear_end = std::chrono::steady_clock::now();

    v.resize(N);

    random_fill(v);

    auto square_start = std::chrono::steady_clock::now();
    square(v);
    auto square_end = std::chrono::steady_clock::now();

    auto iter_duration = (square_end - square_start) + (clear_end - clear_start);

    SET_ITERATION_TIME(iter_duration);
    
    HOTLOOPEND;
}

int main(int argc, char **argv)
{
    comppare::InputContext<>::OutputContext<std::vector<int>> compare;

    compare.set_reference("All", bench_all).google_benchmark()
                                            ->Unit(benchmark::kMicrosecond);

    compare.add("Resize", bench_resize).google_benchmark()
                                        ->Unit(benchmark::kMicrosecond)
                                        ->UseManualTime();

    compare.add("Random Fill", bench_randomfill).google_benchmark()
                                                    ->Unit(benchmark::kMicrosecond)
                                                    ->UseManualTime();

    compare.add("Clear + Square", bench_clear_square).google_benchmark()
                                                    ->Unit(benchmark::kMicrosecond)
                                                    ->UseManualTime();

    compare.run(argc, argv);

    return 0;
}