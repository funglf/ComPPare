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

    // TODO: Merge this into GoogleBenchmarkPlugin
    class google_benchmark_manager
    {
    public:
        google_benchmark_manager() = default;
        ~google_benchmark_manager() = default;

        void initialize(int &argc, char **argv)
        {
            auto [tmp_argc, tmp_argv] = gbench_parser_.parse(argc, argv);
            gbench_argc = tmp_argc;
            gbench_argv = tmp_argv;
            print_benchmark_header();

            benchmark::Initialize(&gbench_argc, gbench_argv);
            benchmark::ReportUnrecognizedArguments(gbench_argc, gbench_argv);
        }

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

        void run()
        {
            benchmark::RunSpecifiedBenchmarks();
            benchmark::Shutdown();
        }

    private:
        int gbench_argc;
        char** gbench_argv;
        comppare::plugin::PluginArgParser gbench_parser_{"--gbench"};

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

    template <class InTup, class OutTup>
    class GoogleBenchmarkPlugin final : public Plugin<InTup, OutTup>
    {
        using Self = GoogleBenchmarkPlugin<InTup, OutTup>;
        comppare::plugin::google_benchmark::google_benchmark_manager gb_;

    public:
        GoogleBenchmarkPlugin(const GoogleBenchmarkPlugin &) = delete;
        GoogleBenchmarkPlugin &operator=(const GoogleBenchmarkPlugin &) = delete;

        static std::shared_ptr<Self> instance()
        {
            static std::shared_ptr<Self> inst{new Self};
            return inst;
        }

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

        void initialize(int &argc, char **argv) override
        {
            gb_.initialize(argc, argv);
        }
        void run() override
        {
            gb_.run();
        }

    private:
        GoogleBenchmarkPlugin() = default;
    };

    template <comppare::internal::concepts::FloatingPoint T>
    inline void SetIterationTime(T time)
    {
        benchmark::State &st = comppare::plugin::google_benchmark::state::get_state();
        st.SetIterationTime(static_cast<double>(time * 1e-6));
    }

    template <typename Rep, typename Period>
    inline void SetIterationTime(std::chrono::duration<Rep, Period> time)
    {
        benchmark::State &st = comppare::plugin::google_benchmark::state::get_state();
        double elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(time).count();
        st.SetIterationTime(elapsed_seconds);
    }

}

#define PLUGIN_HOTLOOP_BENCH                                                       \
    benchmark::State &st = comppare::plugin::google_benchmark::state::get_state(); \
    for (auto _ : st)                                                              \
    {                                                                              \
        hotloop_body();                                                            \
    }

#define GPU_COMPPARE_HOTLOOP_BENCH(prefix)                                         \
    prefix##Event_t __LINE__stop;                                                  \
    prefix##EventCreate(&__LINE__stop);                                            \
    benchmark::State &st = comppare::plugin::google_benchmark::state::get_state(); \
    for (auto _ : st)                                                              \
    {                                                                              \
        hotloop_body();                                                            \
        /* Syncronise every time to record GPU time */                             \
        /* Google Benchmark records extra overhead of EventSynchronization */      \
        /* Google Benchmark not recommended for GPU code anyways. */               \
        prefix##EventRecord(__LINE__stop);                                         \
        prefix##EventSynchronize(__LINE__stop);                                    \
    }

#define PLUGIN_SET_ITERATION_TIME(TIME) \
    comppare::plugin::google_benchmark::SetIterationTime(TIME);

#endif // HAVE_GOOGLE_BENCHMARK