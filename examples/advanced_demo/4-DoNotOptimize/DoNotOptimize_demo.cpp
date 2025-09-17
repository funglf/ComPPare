#include <vector>
#include <numeric>

#include <comppare/comppare.hpp>

#define N 100000

void SAXPY(const float a, const float* x, const float* y)
{
    float yout;
    HOTLOOPSTART;
    for (int i = 0; i < N; ++i)
    {
        yout = a * x[i] + y[i];
    }
    HOTLOOPEND;
}

void SAXPY_DONOTOPTIMIZE(const float a, const float* x, const float* y)
{
    float yout;
    HOTLOOPSTART;
    for (int i = 0; i < N; ++i)
    {
        yout = a * x[i] + y[i];
        comppare::DoNotOptimize(yout);
    }
    HOTLOOPEND;
}

int main(int argc, char **argv)
{
    float a = 2.0f;
    float* x = new float[N];
    float* y = new float[N];
    std::iota(x, x + N, 1.0f);
    std::iota(y, y + N, 1.0f);

    auto compare = comppare::make_comppare<>(a, x, y);

#ifdef HAVE_GOOGLE_BENCHMARK
    compare.set_reference("SAXPY", SAXPY).google_benchmark();
    compare.add("SAXPY_DONOTOPTIMIZE", SAXPY_DONOTOPTIMIZE).google_benchmark();
#else
    compare.set_reference("SAXPY", SAXPY);
    compare.add("SAXPY_DONOTOPTIMIZE", SAXPY_DONOTOPTIMIZE);
#endif

    compare.run(argc, argv);

    delete[] x;
    delete[] y;
}