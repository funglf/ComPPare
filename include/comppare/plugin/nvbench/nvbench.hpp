/*

Copyright 2025 | Leong Fan FUNG | funglf | stanleyfunglf@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

/**
 * @file nvbench.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file contains the NVBench plugin for the ComPPare framework.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 */

#pragma once
#ifdef HAVE_NV_BENCH
#include <nvbench/nvbench.cuh>
#include "comppare/plugin/plugin.hpp"

#include <memory>
#include <functional>
#include <sstream>
#include <string>

namespace comppare::plugin::nvbenchplugin
{
    /**
     * @brief Wrapper for a callable object to be used within NVBench.
     *
     * @tparam F The type of the callable object.
     */
    template <typename F>
    struct NvbenchCallable
    {
        F f;

        void operator()(nvbench::state &st, nvbench::type_list<>)
        {
            f(st);
        }

        NvbenchCallable(F _f) : f(std::move(_f)) {}
        ~NvbenchCallable() = default;

        NvbenchCallable(const NvbenchCallable &other) = default;
        NvbenchCallable &operator=(const NvbenchCallable &other) = default;
        NvbenchCallable(NvbenchCallable &&other) = default;
        NvbenchCallable &operator=(NvbenchCallable &&other) = default;
    };

    /**
     * @brief State class to manage the benchmark state.
     *
     * This class is a singleton to ensure a single instance throughout the program.
     * It provides methods to set and get the benchmark state `nvbench::state *`.
     * The state is used to interact with the NVBench library.
     *
     * @note nvbench uses reference to state. However, move and copy constructor of nvbench::state are deleted.
     *       Therefore, we store a pointer to nvbench::state instead.
     */
    class state
    {
    public:
        state(const state &) = delete;
        state &operator=(const state &) = delete;
        state(state &&) = delete;
        state &operator=(state &&) = delete;

        static state &instance()
        {
            static state inst;
            return inst;
        }

        static void set_state(nvbench::state *st)
        {
            instance().st_ = st;
        }

        static nvbench::state *get_state()
        {
            if (!instance().st_)
                throw std::runtime_error("Benchmark state is not set.");
            return instance().st_;
        }

    private:
        state() = default;
        nvbench::state *st_ = nullptr;
    };

    /**
     * @brief Manager class for NVBench integration.
     *
     * This class handles the initialization, registration, and execution of benchmarks
     * using the NVBench library. It provides methods to initialize the library
     * with command-line arguments, register benchmark functions, and run the benchmarks.
     */
    class nvbench_manager
    {
    public:
        nvbench_manager() = default;
        ~nvbench_manager() = default;

        /**
         * @brief Initialize the NVBench library.
         *
         * @param argc The number of command-line arguments.
         * @param argv The command-line arguments.
         *
         * This function initializes the NVBench library with the provided
         * command-line arguments. It also prints the benchmark header and recognized
         * arguments to the console.
         */
        void initialize(int &argc, char **argv)
        {
            auto [tmp_argc, tmp_argv] = nvbench_parser_.parse(argc, argv);
            nvbench_argc = tmp_argc;
            nvbench_argv = tmp_argv;
            print_benchmark_header();
        }

        /**
         * @brief Register a benchmark function with NVBench.
         *
         * @tparam Func The type of the benchmark function.
         * @tparam Args The types of the arguments to the benchmark function.
         * @param name The name of the benchmark.
         * @param f The function/implementation to register.
         * @param args The arguments to pass to the benchmark function.
         * @return A reference to the registered benchmark.
         *
         * This helper simplifies registering a benchmark by automatically wrapping the
         * provided function and its arguments into a NVBench-compatible lambda.
         * The lambda sets the current benchmark state and invokes the function with the
         * captured arguments.
         *
         * Usage example:
         * ```cpp
         * void saxpy(int n, float a, const float* x, float* y);
         *
         * // Register benchmark: equivalent to writing a manual BM_ function
         * add_nvbench("SAXPY", saxpy, 1<<20, 2.0f, x_data, y_data);
         * ```
         *
         * This is equivalent to writing a manual NVBench function:
         * ```cpp
         * static void BM_SAXPY(nvbench::state &state) {
         *   // setup code here 
         *   state.exec([&](nvbench::launch& launch) {
         *     saxpy<<<numBlocks, blockSize>>>(a, d_x, d_y, n);
         *   });
         * }
         * NVBENCH_BENCH(BM_SAXPY);
         * ```
         */
        template <typename Func, typename... Args>
        nvbench::benchmark_base &add_nvbench(const char *name, Func f, Args &&...args)
        {
            std::tuple<std::decay_t<Args>...> cargs(std::forward<Args>(args)...);

            auto nvbench_wrapper = [f, cargs = std::move(cargs)](nvbench::state &st) mutable
            {
                comppare::plugin::nvbenchplugin::state::set_state(&st);
                std::apply([&](auto &&...unpacked)
                           { f(std::forward<decltype(unpacked)>(unpacked)...); }, cargs);
            };

            using Callable = NvbenchCallable<decltype(nvbench_wrapper)>;

            return nvbench::benchmark_manager::get()
                .add(std::make_unique<nvbench::benchmark<Callable>>(Callable{std::move(nvbench_wrapper)}))
                .set_name(name);
        }

        /**
         * @brief Run the registered benchmarks.
         *
         * This function runs all benchmarks that have been registered with the
         * NVBench library. It should be called after all benchmarks have
         * been registered and the library has been initialized.
         */
        void run()
        {
            NVBENCH_MAIN_INITIALIZE(nvbench_argc, nvbench_argv);
            {
                NVBENCH_MAIN_PARSE(nvbench_argc, nvbench_argv);

                NVBENCH_MAIN_PRINT_PREAMBLE(parser);
                NVBENCH_MAIN_RUN_BENCHMARKS(parser);
                NVBENCH_MAIN_PRINT_EPILOGUE(parser);

                NVBENCH_MAIN_PRINT_RESULTS(parser);
            } /* Tear down parser before finalization */
            NVBENCH_MAIN_FINALIZE();
        }

    private:
        int nvbench_argc;
        char **nvbench_argv;
        /** @brief Argument parser for NVBench. */
        comppare::plugin::PluginArgParser nvbench_parser_{"--nvbench"};

        /** @brief Print the benchmark header. */
        void print_benchmark_header()
        {

            std::cout << "\n"
                      << std::left << comppare::internal::ansi::BOLD
                      << "*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*\n============= "
                      << comppare::internal::ansi::ITALIC("nvbench")
                      << " =============\n*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*"
                      << comppare::internal::ansi::BOLD_OFF << "\n\n";

            std::cout << "nvbench cmdline arguments:\n";
            for (int i = 0; i < nvbench_argc; ++i)
            {
                std::cout << std::setw(2) << std::right << " "
                          << "  [" << i << "] " << std::quoted(nvbench_argv[i]) << "\n";
            }

            std::cout << std::left
                      << comppare::internal::ansi::BOLD("*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*")
                      << "\n\n";
        }
    };

    /**
     * @brief NVBench plugin for the ComPPare framework.
     *
     * This plugin integrates the NVBench library into the ComPPare framework.
     * It provides methods to register implementations, initialize the benchmark library,
     * and run the benchmarks. The plugin is designed to work with input and output tuples
     * for flexible benchmarking of various functions.
     *
     * @tparam InTup The type of the input tuple.
     * @tparam OutTup The type of the output tuple.
     */
    template <class InTup, class OutTup>
    class nvbenchPlugin final : public Plugin<InTup, OutTup>
    {
        using Self = nvbenchPlugin<InTup, OutTup>;

    private:
        nvbenchPlugin() = default;
        comppare::plugin::nvbenchplugin::nvbench_manager nb_;

    public:
        nvbenchPlugin(const nvbenchPlugin &) = delete;
        nvbenchPlugin &operator=(const nvbenchPlugin &) = delete;
        
        /**
         * @brief Get the singleton instance of the nvbenchPlugin.
         *
         * @return A shared pointer to the singleton instance.
         *
         * This method ensures that only one instance of the nvbenchPlugin
         * exists throughout the program. It returns a shared pointer to the instance.
         */
        static std::shared_ptr<Self> instance()
        {
            static std::shared_ptr<Self> inst{new Self};
            return inst;
        }

        /**
         * @brief Register an implementation with the NVBench plugin.
         *
         * @tparam Func The type of the user-provided function.
         * @param name The name of the implementation.
         * @param user_fn The function/implementation to register.
         * @param inputs A const reference to the input tuple.
         * @param outs A reference to the output tuple.
         * @return A pointer to the registered benchmark.
         *
         * This method passes the user-provided function and its arguments to the
         * internal nvbench_manager for registration. It uses `std::apply`
         * to unpack the input and output tuples and forward them to the manager's
         * `add_nvbench` method.
         */
        template <class Func>
        nvbench::benchmark_base &register_impl(const std::string &name,
                                               Func &&user_fn,
                                               const InTup &inputs,
                                               OutTup &outs)
        {
            return std::apply([&](auto &&...in_vals) -> nvbench::benchmark_base &
                              { return std::apply([&](auto &&...out_vals) -> nvbench::benchmark_base &
                                                  { return nb_.add_nvbench(name.c_str(),
                                                                           std::forward<Func>(user_fn),
                                                                           std::forward<decltype(in_vals)>(in_vals)...,
                                                                           std::forward<decltype(out_vals)>(out_vals)...); }, outs); }, inputs);
        }

        /** @brief Initialize the NVBench plugin.
         *
         * This method initializes the NVBench plugin by calling the
         * `initialize` method of the internal nvbench_manager.
         *
         * @param argc The number of command-line arguments.
         * @param argv The command-line arguments.
         */
        void initialize(int &argc, char **argv) override
        {
            nb_.initialize(argc, argv);
        }

        /** @brief Run the registered benchmarks.
         *
         * This method runs the benchmarks by calling the `run` method of the
         * internal nvbench_manager.
         */
        void run() override
        {
            nb_.run();
        }
    };
}

/**
 * @brief Macro to benchmark a function using NVBench.
 */
#define PLUGIN_HOTLOOP_BENCH                                           \
    auto state_ = comppare::plugin::nvbenchplugin::state::get_state(); \
    state_->exec([&](nvbench::launch &launch) { hotloop_body(); });

/**
 * @brief Macro to benchmark a GPU function using NVBench.
 */
#define GPU_PLUGIN_HOTLOOP_BENCH PLUGIN_HOTLOOP_BENCH

#endif