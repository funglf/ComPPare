#include "max_cpu.hpp"
#include "init_max.hpp"

#ifdef HAVE_CUDA
#include "max_gpu.cuh"
#endif

#include <comppare/comppare.hpp>

int main(int argc, char **argv)
{
    // Initialize configuration
    MaxConfig cfg = init_max(argc, argv);

    // Using make_comppare helper to create a comppare instance
    // comppare::make_comppare<output_types>(input_args);
    // equivalent to comppare::InputContext<std::span<const float>>::OutputContext<float>
    auto compare = comppare::make_comppare<float>(cfg.data);

    // Set reference implementation
    compare.set_reference("cpu serial", cpu_max_serial);
    // Add implementations to compare
    compare.add("cpu omp", cpu_max_omp);
    compare.add("cpu thread", cpu_max_thread);
#ifdef HAVE_CUDA
    compare.add("gpu kernel", gpu_max<max_kernel>);
    compare.add("gpu kernel opt", gpu_max<max_kernel_warpsemantics>);
#endif

    // Run the comparison with specified iterations and tolerance
    compare.run(argc, argv);

    return 0;
}