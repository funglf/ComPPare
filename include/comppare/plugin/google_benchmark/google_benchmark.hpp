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
 * @file google_benchmark.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file contains the Google Benchmark plugin for the ComPPare framework.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 */

#pragma once
#ifdef HAVE_GOOGLE_BENCHMARK
#include <utility>
#include <tuple>
#include <ostream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cstring>
#include <benchmark/benchmark.h>

#include "comppare/plugin/plugin.hpp"
#include "comppare/internal/ansi.hpp"

namespace comppare::plugin::google_benchmark
{
    /**
     * @brief State class to manage the benchmark state.
     * 
     * This class is a singleton to ensure a single instance throughout the program.
     * It provides methods to set and get the benchmark state `benchmark::State &`.
     * The state is used to interact with the Google Benchmark library.
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

        static void set_state(benchmark::State &st)
        {
            instance().st_ = &st;
        }

        static benchmark::State &get_state()
        {
            if (!instance().st_)
                throw std::runtime_error("Benchmark state is not set.");
            return *instance().st_;
        }

    private:
        state() = default;
        benchmark::State *st_ = nullptr;
    };

    /**
     * @brief Manager class for Google Benchmark integration.
     *
     * This class handles the initialization, registration, and execution of benchmarks
     * using the Google Benchmark library. It provides methods to initialize the library
     * with command-line arguments, register benchmark functions, and run the benchmarks.
     */
    class google_benchmark_manager
    {
    public:
        google_benchmark_manager() = default;
        ~google_benchmark_manager() = default;

        /**
         * @brief Initialize the Google Benchmark library.
         *
         * @param argc The number of command-line arguments.
         * @param argv The command-line arguments.
         *
         * This function initializes the Google Benchmark library with the provided
         * command-line arguments. It also prints the benchmark header and recognized
         * arguments to the console.
         */
        void initialize(int &argc, char **argv)
        {
            auto [tmp_argc, tmp_argv] = gbench_parser_.parse(argc, argv);
            gbench_argc = tmp_argc;
            gbench_argv = tmp_argv;
            print_benchmark_header();

            benchmark::Initialize(&gbench_argc, gbench_argv);
            benchmark::ReportUnrecognizedArguments(gbench_argc, gbench_argv);
        }

        /**
         * @brief Register a benchmark function with Google Benchmark.
         *
         * @tparam Func The type of the benchmark function.
         * @tparam Args The types of the arguments to the benchmark function.
         * @param name The name of the benchmark.
         * @param f The function/implementation to register.
         * @param args The arguments to pass to the benchmark function.
         * @return A pointer to the registered benchmark.
         *
         * This helper simplifies registering a benchmark by automatically wrapping the
         * provided function and its arguments into a Google Benchmark-compatible lambda.
         * The lambda sets the current benchmark state and invokes the function with the
         * captured arguments.
         *
         * Usage example:
         * ```cpp
         * void saxpy(int n, float a, const float* x, float* y);
         *
         * // Register benchmark: equivalent to writing a manual BM_ function
         * add_gbench("SAXPY", saxpy, 1<<20, 2.0f, x_data, y_data);
         * ```
         *
         * This is equivalent to writing a manual Google Benchmark function:
         * ```cpp
         * static void BM_SAXPY(benchmark::State& st) {
         *     for (auto _ : st) {
         *         saxpy(1<<20, 2.0f, x_data, y_data);
         *     }
         * }
         * BENCHMARK(BM_SAXPY);
         * ```
         */
        template <typename Func, typename... Args>
        benchmark::internal::Benchmark *add_gbench(const char *name, Func f, Args &&...args)
        {
            std::tuple<Args...> cargs(std::forward<Args>(args)...);

            auto benchptr = benchmark::RegisterBenchmark(
                name,
                [f, cargs = std::move(cargs)](benchmark::State &st) mutable
                {
                    comppare::plugin::google_benchmark::state::set_state(st);
                    std::apply([&](auto &&...unpacked)
                               { f(std::forward<decltype(unpacked)>(unpacked)...); }, cargs);
                    benchmark::ClobberMemory();
                });

            return benchptr;
        }

        /**
         * @brief Run the registered benchmarks.
         *
         * This function runs all benchmarks that have been registered with the
         * Google Benchmark library. It should be called after all benchmarks have
         * been registered and the library has been initialized.
         */
        void run()
        {
            benchmark::RunSpecifiedBenchmarks();
            benchmark::Shutdown();
        }

    private:
        int gbench_argc;
        char **gbench_argv;
        /** @brief Argument parser for Google Benchmark. */
        comppare::plugin::PluginArgParser gbench_parser_{"--gbench"};

        /** @brief Print the benchmark header. */
        void print_benchmark_header()
        {

            std::cout << "\n"
                      << std::left << comppare::internal::ansi::BOLD
                      << "*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=\n============= "
                      << comppare::internal::ansi::ITALIC("Google Benchmark")
                      << " =============\n=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*"
                      << comppare::internal::ansi::BOLD_OFF << "\n\n";

            std::cout << "Google Benchmark cmdline arguments:\n";
            for (int i = 0; i < gbench_argc; ++i)
            {
                std::cout << std::setw(2) << std::right << " "
                          << "  [" << i << "] " << std::quoted(gbench_argv[i]) << "\n";
            }

            std::cout << std::left
                      << comppare::internal::ansi::BOLD("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*")
                      << "\n\n";
        }
    };

    /**
     * @brief Google Benchmark plugin for the ComPPare framework.
     *
     * This plugin integrates the Google Benchmark library into the ComPPare framework.
     * It provides methods to register implementations, initialize the benchmark library,
     * and run the benchmarks. The plugin is designed to work with input and output tuples
     * for flexible benchmarking of various functions.
     *
     * @tparam InTup The type of the input tuple.
     * @tparam OutTup The type of the output tuple.
     */
    template <class InTup, class OutTup>
    class GoogleBenchmarkPlugin final : public Plugin<InTup, OutTup>
    {
        using Self = GoogleBenchmarkPlugin<InTup, OutTup>;
        comppare::plugin::google_benchmark::google_benchmark_manager gb_;

    public:
        GoogleBenchmarkPlugin(const GoogleBenchmarkPlugin &) = delete;
        GoogleBenchmarkPlugin &operator=(const GoogleBenchmarkPlugin &) = delete;

        /**
         * @brief Get the singleton instance of the GoogleBenchmarkPlugin.
         *
         * @return A shared pointer to the singleton instance.
         *
         * This method ensures that only one instance of the GoogleBenchmarkPlugin
         * exists throughout the program. It returns a shared pointer to the instance.
         */
        static std::shared_ptr<Self> instance()
        {
            static std::shared_ptr<Self> inst{new Self};
            return inst;
        }

        /**
         * @brief Register an implementation with the Google Benchmark plugin.
         *
         * @tparam Func The type of the user-provided function.
         * @param name The name of the implementation.
         * @param user_fn The function/implementation to register.
         * @param inputs A const reference to the input tuple.
         * @param outs A reference to the output tuple.
         * @return A pointer to the registered benchmark.
         *
         * This method passes the user-provided function and its arguments to the
         * internal google_benchmark_manager for registration. It uses `std::apply`
         * to unpack the input and output tuples and forward them to the manager's
         * `add_gbench` method.
         */
        template <class Func>
        benchmark::internal::Benchmark *register_impl(const std::string &name,
                                                      Func &&user_fn,
                                                      const InTup &inputs,
                                                      OutTup &outs)
        {
            return std::apply([&](auto const &...in_vals)
                              { return std::apply([&](auto &&...outs_vals)
                                                  { return gb_.add_gbench(name.c_str(),
                                                                          std::forward<Func>(user_fn),
                                                                          in_vals..., outs_vals...); }, outs); }, inputs);
        }

        /** @brief Initialize the Google Benchmark plugin.
         *
         * This method initializes the Google Benchmark plugin by calling the
         * `initialize` method of the internal google_benchmark_manager.
         *
         * @param argc The number of command-line arguments.
         * @param argv The command-line arguments.
         */
        void initialize(int &argc, char **argv) override
        {
            gb_.initialize(argc, argv);
        }

        /** @brief Run the registered benchmarks.
         *
         * This method runs the benchmarks by calling the `run` method of the
         * internal google_benchmark_manager.
         */
        void run() override
        {
            gb_.run();
        }

    private:
        GoogleBenchmarkPlugin() = default;
    };

    /**
     * @brief Set the iteration time for the current benchmark.
     *
     * @tparam T The type of the time value (must be a floating point type).
     * @param time The iteration time in microseconds.
     *
     * This function sets the iteration time for the current benchmark state.
     * It converts the provided time from microseconds to seconds and calls
     * `SetIterationTime` on the benchmark state.
     * @note This function is only applicable when using Manual timing mode in Google Benchmark.
     * @note This function must be called within the benchmark loop to take effect.
     */
    template <comppare::internal::concepts::FloatingPoint T>
    inline void SetIterationTime(T time)
    {
        benchmark::State &st = comppare::plugin::google_benchmark::state::get_state();
        st.SetIterationTime(static_cast<double>(time * 1e-6));
    }

    /**
     * @brief Set the iteration time for the current benchmark.
     *
     * @tparam Rep The representation type of the duration.
     * @tparam Period The period type of the duration.
     * @param time The iteration time as a `std::chrono::duration`.
     *
     * This function sets the iteration time for the current benchmark state.
     * It converts the provided `std::chrono::duration` to seconds and calls
     * `SetIterationTime` on the benchmark state.
     * @note This function is only applicable when using Manual timing mode in Google Benchmark.
     * @note This function must be called within the benchmark loop to take effect.
     */
    template <typename Rep, typename Period>
    inline void SetIterationTime(std::chrono::duration<Rep, Period> time)
    {
        benchmark::State &st = comppare::plugin::google_benchmark::state::get_state();
        double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(time).count();
        st.SetIterationTime(elapsed_seconds);
    }

}

/**
 * @brief Macro to benchmark a function using Google Benchmark.
 */
#define PLUGIN_HOTLOOP_BENCH                                                       \
    benchmark::State &st = comppare::plugin::google_benchmark::state::get_state(); \
    for (auto _ : st)                                                              \
    {                                                                              \
        hotloop_body();                                                            \
    }

#if defined(__CUDACC__)
#define GPU_PLUGIN_HOTLOOP_BENCH                                                   \
    cudaEvent_t __LINE__stop;                                                      \
    cudaEventCreate(&__LINE__stop);                                                \
    benchmark::State &st = comppare::plugin::google_benchmark::state::get_state(); \
    for (auto _ : st)                                                              \
    {                                                                              \
        hotloop_body();                                                            \
        /* Syncronise every time to record GPU time */                             \
        /* Google Benchmark records extra overhead of EventSynchronization */      \
        /* Google Benchmark not recommended for GPU code anyways. */               \
        cudaEventRecord(__LINE__stop);                                             \
        cudaEventSynchronize(__LINE__stop);                                        \
    }
#elif defined(__HIPCC__)
#define GPU_PLUGIN_HOTLOOP_BENCH                                                   \
    hipEvent_t __LINE__stop;                                                       \
    hipEventCreate(&__LINE__stop);                                                 \
    benchmark::State &st = comppare::plugin::google_benchmark::state::get_state(); \
    for (auto _ : st)                                                              \
    {                                                                              \
        hotloop_body();                                                            \
        /* Syncronise every time to record GPU time */                             \
        /* Google Benchmark records extra overhead of EventSynchronization */      \
        /* Google Benchmark not recommended for GPU code anyways. */               \
        hipEventRecord(__LINE__stop);                                              \
        hipEventSynchronize(__LINE__stop);                                         \
    }
#endif

/** @brief Macro to set the iteration time for a benchmark. */
#define PLUGIN_SET_ITERATION_TIME(TIME) \
    comppare::plugin::google_benchmark::SetIterationTime(TIME);

#endif // HAVE_GOOGLE_BENCHMARK