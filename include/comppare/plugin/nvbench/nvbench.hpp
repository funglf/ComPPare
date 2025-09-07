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
#ifdef HAVE_NV_BENCH
#include <nvbench/nvbench.cuh>
#include "comppare/plugin/plugin.hpp"

#include <memory>
#include <functional>
#include <sstream>
#include <string>

namespace comppare::plugin::nvbenchplugin
{
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

    class nvbench_manager
    {
    public:
        nvbench_manager() = default;
        ~nvbench_manager() = default;

        void initialize(int &argc, char **argv)
        {
            auto [tmp_argc, tmp_argv] = nvbench_parser_.parse(argc, argv);
            nvbench_argc = tmp_argc;
            nvbench_argv = tmp_argv;
            print_benchmark_header();
        }

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
        char** nvbench_argv;
        comppare::plugin::PluginArgParser nvbench_parser_{"--nvbench"};

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

        static std::shared_ptr<Self> instance()
        {
            static std::shared_ptr<Self> inst{new Self};
            return inst;
        }

        template <class Func>
        nvbench::benchmark_base &register_impl(const std::string &name,
                                               Func &&user_fn,
                                               const InTup &inputs,
                                               OutTup &outs)
        {
            return std::apply([&](auto&&... in_vals) -> nvbench::benchmark_base& {
                return std::apply([&](auto&&... out_vals) -> nvbench::benchmark_base& {
                    return nb_.add_nvbench(name.c_str(),
                                            std::forward<Func>(user_fn),
                                            std::forward<decltype(in_vals)>(in_vals)...,
                                            std::forward<decltype(out_vals)>(out_vals)...);
                }, outs);
            }, inputs);
        }

        void initialize(int &argc, char **argv) override
        {
            nb_.initialize(argc, argv);
        }
        void run() override
        {
            nb_.run();
        }
    };
}

#define PLUGIN_HOTLOOP_BENCH                                           \
    auto state_ = comppare::plugin::nvbenchplugin::state::get_state(); \
    state_->exec([&](nvbench::launch &launch) { hotloop_body(); });

#define GPU_PLUGIN_HOTLOOP_BENCH PLUGIN_HOTLOOP_BENCH

#endif