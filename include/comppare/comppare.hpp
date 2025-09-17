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
 * @file comppare.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file is the main include file for the ComPPare framework.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 */

#pragma once
#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <string_view>
#include <concepts>
#include <stdexcept>

#include <comppare/internal/ansi.hpp>
#include <comppare/internal/config.hpp>
#include <comppare/internal/helper.hpp>
#include <comppare/internal/policy.hpp>
#include <comppare/plugin/plugin.hpp>

#if defined(HAVE_GOOGLE_BENCHMARK) && defined(HAVE_NV_BENCH)
#warning "Please only use one Plugin."
#endif

#if defined(HAVE_GOOGLE_BENCHMARK)
#include "comppare/plugin/google_benchmark/google_benchmark.hpp"
#endif

#if defined(HAVE_NV_BENCH)
#include "comppare/plugin/nvbench/nvbench.hpp"
#endif

/**
 * @brief ComPPare framework main namespace.
 * @namespace comppare
 */
namespace comppare
{
    /*
    DoNotOptimize() and ClobberMemory() are utility functions to prevent compiler optimizations

    Reference:
    CppCon 2015: Chandler Carruth "Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!"
    Google Benchmark: https://github.com/google/benchmark

    Copyright 2015 Google Inc. All rights reserved.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
    */

    /**
     * @brief Prevents the compiler from optimizing away the given value.
     *
     * @tparam T The type of the value to protect from optimization.
     * @param value The value to protect from optimization.
     *
     * This implementation is verbatim from Google Benchmark’s benchmark::DoNotOptimize(), licensed under Apache 2.0. No changes have been made.
     */
    template <typename T>
    inline __attribute__((always_inline)) void DoNotOptimize(T const &value)
    {
        asm volatile("" : : "r,m"(value) : "memory");
    }

    /**
     * @brief Prevents the compiler from optimizing away the given value.
     *
     * @tparam T The type of the value to protect from optimization.
     * @param value The value to protect from optimization.
     *
     * This implementation is verbatim from Google Benchmark’s benchmark::DoNotOptimize(), licensed under Apache 2.0. No changes have been made.
     */
    template <typename T>
    inline __attribute__((always_inline)) void DoNotOptimize(T &value)
    {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }

    /**
     * @brief Prevents the compiler from optimizing away the given value.
     *
     * @tparam T The type of the value to protect from optimization.
     * @param value The value to protect from optimization.
     *
     * This implementation is verbatim from Google Benchmark’s benchmark::DoNotOptimize(), licensed under Apache 2.0. No changes have been made.
     */
    template <typename T>
    inline __attribute__((always_inline)) void DoNotOptimize(T &&value)
    {
#if defined(__clang__)
        asm volatile("" : "+r,m"(value) : : "memory");
#else
        asm volatile("" : "+m,r"(value) : : "memory");
#endif
    }

    // This implementation is verbatim from Google Benchmark’s benchmark::ClobberMemory(),
    // licensed under Apache 2.0. No changes have been made.
    inline __attribute__((always_inline)) void ClobberMemory()
    {
        std::atomic_signal_fence(std::memory_order_acq_rel);
    }

    /**
     * @brief Specification struct for pairing an output type with an error policy.
     *
     * The `outspec` struct represents an output type together with its error policy.
     * Inside the framework, every argument passed to `OutputContext` is normalized
     * into some form of `outspec<Value, Policy>`.
     *
     * ### How `OutputContext` normalizes output types
     * - **Raw types**
     *   If the user passes a bare type (e.g. `double`), it is automatically
     *   wrapped as `outspec<double, void>`.
     *   The `void` signals that the framework should deduce the policy
     *   via `AutoPolicy_t<double>`.
     *   This case is handled by the specialization `outspec<Value, void>`.
     *
     * - **Explicit policies**
     *   If the user passes `set_policy<T, P>`, it is an alias for `outspec<T, P>`.
     *   Within `OutputContext`, this is first normalized as `outspec<outspec<T, P>, void>`,
     *   in the same way raw types are normalized.
     *   This case is handled by the specialization `outspec<outspec<Value, Policy>, void>`.
     *
     * ### Why the double-wrapping
     * Consider a mix of raw types and explicit policies in `OutputContext`:
     * ```cpp
     * comppare::InputContext<>::OutputContext<outspec<T0, P0>, T1>
     * ```
     * Here:
     * - `T1` is normalized as `outspec<T1, void>` (auto policy).
     * - `outspec<T0, P0>` is normalized as `outspec<outspec<T0, P0>, void>`.
     *
     * *All arguments are first treated as `outspec<..., void>`*.
     * Without this mechanism, users would have to explicitly write:
     * ```cpp
     * comppare::InputContext<>::OutputContext<outspec<T0, P0>, outspec<T1, void>>
     * ```
     * which is less convenient, since most users do not need custom policies.
     *
     * @tparam Value  The output value type.
     * @tparam Policy The error policy type. Defaults to `void` for auto-selection.
     */
    template <typename Value, typename Policy = void>
    struct outspec;

    /**
     * @brief Partial specialization of outspec for automatic error policy selection.
     * 
     * This specialization is used when the user provides only a value type:
     * ```cpp
     * comppare::InputContext<>::OutputContext<double, std::string>
     * ```
     * 
     * Therefore, the framework normalises each output type to a `outspec<Value, void>`:
     * ```cpp
     * comppare::InputContext<>::OutputContext<outspec<double, void>, outspec<std::string, void>>
     * ```
     * 
     * This specialization matches `outspec<Value, void>` and uses `AutoPolicy_t<Value>` to
     * deduce the error policy for the given value type.
     *
     * @note See documentation of `outspec` for greater overview on the design choices.
     * @see outspec for greater overview on the design choices.
     * @tparam Value
     */
    template <typename Value>
        requires internal::policy::autopolicy::SupportedByAutoPolicy<Value>
    struct outspec<Value, void>
    {
        using outtype_t = std::decay_t<Value>;
        using policy_t = internal::policy::autopolicy::AutoPolicy_t<Value>;
    };

    /**
     * @brief Partial specialization of outspec for user-defined error policy selection.
     * 
     * This specialization is used when the user provides both a value type and an explicit policy:
     * ```cpp
     * comppare::InputContext<>::OutputContext<outspec<double, MyDoublePolicy>, outspec<std::string, MyStringPolicy>>
     * ```
     * Therefore, the framework normalises each output type to a `outspec<outspec<Value, Policy>, void>`:
     * ```cpp
     * comppare::InputContext<>::OutputContext<outspec<outspec<double, MyDoublePolicy>, void>, outspec<outspec<std::string, MyStringPolicy>, void>>
     * ```
     * This specialization matches `outspec<outspec<Value, Policy>, void>` and extracts the user-provided
     * value and policy types.
     * 
     * @note See documentation of `outspec` for for greater overview on the design choices.
     * @see outspec for greater overview on the design choices.
     * @tparam Value
     * @tparam Policy
     */
    template <typename Value, typename Policy>
    struct outspec<outspec<Value, Policy>, void>
    {
        using outtype_t = std::decay_t<Value>;
        using policy_t = Policy;
    };

    /**
     * @brief Partial specialization of outspec for user-defined error policy selection.
     * 
     * @note This specialisation is not strictly required in this context as `outspec<outspec<T, P>, void>` would be sufficient.
     * However, it is included for clarity and to explicitly handle the case where both Value and Policy are provided.
     * @see outspec
     * @tparam Value
     * @tparam Policy
     */
    template <typename Value, typename Policy>
        requires comppare::internal::policy::ErrorPolicy<Value, Policy>
    struct outspec<Value, Policy>
    {
        using outtype_t = std::decay_t<Value>;
        using policy_t = Policy;
    };

    /** @brief Alias for setting the error policy for a type. */
    template <typename Value, typename Policy>
    using set_policy = outspec<Value, Policy>;

    /** @brief Concept for output specifications being pair of type and policy. */
    template <typename T>
    concept OutSpec =
        comppare::internal::policy::ErrorPolicy<
            typename outspec<T>::outtype_t,
            typename outspec<T>::policy_t>;

    /**
     * @brief InputContext class template to hold input parameters for the comparison framework.
     *
     * @tparam Inputs
     */
    template <typename... Inputs>
    class InputContext
    {
    public:
        /**
         * @brief OutputContext class template to hold output parameters and manage implementations.
         *
         * @tparam OutputSpecs
         */
        template <OutSpec... OutputSpecs>
        class OutputContext
        {
        private:
            /**
             * @tparam S The output outspec type.
             * @brief Extracts the value type from a outspec.
             */
            template <typename S>
            using outtype_t = typename outspec<S>::outtype_t;
            /**
             * @tparam S The output outspec type.
             * @brief Extracts the policy type from a outspec.
             */
            template <typename S>
            using pol_t = typename outspec<S>::policy_t;

            /**
             * @brief Alias for the function signature of a user-provided implementation.
             *
             * The function must take all input arguments by const reference, and
             * all output arguments by non-const reference. The framework invokes
             * this function to compare multiple implementations on the same input.
             *
             * Example signature:
             * @code
             * void f(const In1&, const In2&, ..., Out1&, Out2&...);
             * @endcode
             */
            using Func = std::function<void(const std::decay_t<Inputs> &..., outtype_t<OutputSpecs> &...)>;

            /**
             * @brief Tuple type holding all input arguments.
             */
            using InTup = std::tuple<std::decay_t<Inputs>...>;
            /**
             * @brief Tuple type holding all output values (one element per outspec).
             */
            using OutTup = std::tuple<outtype_t<OutputSpecs>...>;
            /**
             * @brief Tuple type holding the error/policy object associated with each output outspec.
             */
            using PolicyTup = std::tuple<pol_t<OutputSpecs>...>;

            /**
             * @brief Shared pointer to an output tuple.
             *
             * Used to manage lifetime of output results across multiple implementations.
             */
            using OutPtr = std::shared_ptr<OutTup>;

            /**
             * @brief Tuple instance storing all current input arguments.
             */
            InTup inputs_;
            /**
             * @brief Storage for reference and comparison outputs.
             *
             * Each implementation’s outputs are stored here as shared pointer to tuples.
             * The first implementation is treated as the reference.
             */
            std::vector<OutPtr> outputs_;
            /**
             * @brief Tuple of policy objects for the reference outputs.
             *
             * Each policy governs how the corresponding output is validated
             */
            PolicyTup policies_ref_;

            /**
             * @brief Number of output specifications.
             *
             * This is equal to the number of outputs of a function.
             *
             * @see https://en.cppreference.com/w/cpp/language/sizeof....html
             */
            static constexpr size_t NUM_OUT = sizeof...(OutputSpecs);

            /**
             * @brief Shared pointer to the plugin instance.
             *
             * This pointer is used to store the plugin instance and ensure only one plugin is registered.
             * It is shared across all implementations within the output context.
             */
            std::shared_ptr<plugin::Plugin<InTup, OutTup>> plugin_;

            /**
             * @brief Register a plugin for the output context.
             *
             * @param p The shared pointer to the plugin to register.
             */
            void register_plugin(const std::shared_ptr<plugin::Plugin<InTup, OutTup>> &p)
            {
                if (!plugin_)
                    plugin_ = p;
                else if (plugin_ != p)
                    throw std::logic_error("Multiple plugins are not supported");
            }

            /**
             * @struct Impl
             * @brief Internal container representing one registered implementation.
             *
             * Each `Impl` bundles together:
             *   - the *user function* (`fn`) under a given name,
             *   - a pointer to the input tuple,
             *   - a back-reference to the owning `OutputContext`,
             *   - and optionally, plugin-managed output storage.
             *
             * This allows the framework to keep track of multiple competing
             * implementations of the same operation (e.g. reference vs optimized),
             * and to attach correctness/performance plugins such as Google Benchmark
             * or NVBench.
             *
             */
            struct Impl
            {
                /**
                 * @brief Name of the implementation.
                 *
                 * This is used to identify the implementation in logs and reports.
                 */
                std::string name;
                /**
                 * @brief The user-provided function implementing the operation.
                 *
                 * This function must match the signature defined by `Func`, taking
                 * all input arguments by const reference, and all output arguments
                 * by non-const reference.
                 */
                Func fn;

                /**
                 * @brief Pointer to the input tuple `inputs_`.
                 */
                InTup *inputs_ptr;
                /**
                 * @brief Reference to the owning `OutputContext`.
                 * This allows the implementation to register plugins.
                 */
                OutputContext *parent_ctx;

                /**
                 * @brief Unique pointer to the output tuple for plugin runs.
                 *
                 * This allows the implementation to provide a separate output
                 * instance for plugins, avoiding interference with the main
                 * output.
                 */
                std::unique_ptr<OutTup> plugin_output = nullptr;

#ifdef HAVE_GOOGLE_BENCHMARK
                /**
                 * @brief Attach the Google Benchmark plugin.
                 *
                 * This function adds the Google Benchmark plugin for the current implementation.
                 */
                decltype(auto) google_benchmark()
                {
                    return attach<plugin::google_benchmark::GoogleBenchmarkPlugin>();
                }
#endif

#ifdef HAVE_NV_BENCH
                /**
                 * @brief Attach the nvbench plugin.
                 *
                 * This function adds the nvbench plugin for the current implementation.
                 */
                decltype(auto) nvbench()
                {
                    return attach<plugin::nvbenchplugin::nvbenchPlugin>();
                }
#endif

                /**
                 * @brief Attach a plugin to the output context.
                 *
                 * @tparam Plugin The plugin type to attach.
                 * @return The return of the plugin's `register_impl` method.
                 *
                 * @note decltype(auto) is used to preserve the return type of the plugin's register_impl method.
                 * When using `auto`, the return type would remove its reference-ness and const-ness; while decltype(auto) preserves it.
                 * Reference: Effective Modern C++ Item 3: Understand decltype
                 */
                template <template <class, class> class Plugin>
                    requires comppare::plugin::ValidPlugin<Plugin, InTup, OutTup, Func>
                decltype(auto) attach()
                {
                    auto adp = Plugin<InTup, OutTup>::instance();

                    parent_ctx->register_plugin(adp);

                    plugin_output = std::make_unique<OutTup>();

                    return adp->register_impl(name, fn, *inputs_ptr, *plugin_output);
                }
            };

            /**
             * @brief Vector to hold all implementations.
             *
             * This vector is used to store all the different implementations
             * of the operation being benchmarked.
             */
            std::vector<Impl> impls_;

            // helpers -----------------------------------------------------------
            /**
             * @brief Get the implementation details for a specific implementation index.
             *
             * @tparam I The implementation index.
             * @return std::size_t The number of metrics for the implementation.
             */
            template <std::size_t I>
            static constexpr std::size_t spec_metric_count() { return std::tuple_element_t<I, PolicyTup>::metric_count(); }

            /**
             * @brief Get the name of a specific metric for a specific implementation index.
             *
             * @tparam I The implementation index.
             * @param m The metric index of the output policy.
             * @return std::string_view The name of the metric.
             */
            template <std::size_t I>
            static constexpr std::string_view spec_metric_name(std::size_t m) { return std::tuple_element_t<I, PolicyTup>::metric_name(m); }

            /**
             * @brief Set the width of the print columns.
             */
            static constexpr int PRINT_COL_WIDTH = 20;

            /**
             * @brief Print the header for the output table.
             *
             * This includes the framework title, number of implementations,
             * warmup iterations, and benchmark iterations.
             * It also prints the column headers for the output table.
             * The metric headers are printed dynamically based on the number of output specs.
             */
            void print_header() const
            {
                std::cout << std::left << comppare::internal::ansi::BOLD
                          << "*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=\n============ "
                          << comppare::internal::ansi::ITALIC("ComPPare Framework")
                          << " ============\n=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*"
                          << comppare::internal::ansi::BOLD_OFF << "\n\n";
                std::cout
                    << std::left << std::setw(30) << "Number of implementations: "
                    << std::right << std::setw(10) << impls_.size() << "\n"
                    << std::left << std::setw(30) << "Warmup iterations: "
                    << std::right << std::setw(10) << comppare::config::warmup_iters() << "\n"
                    << std::left << std::setw(30) << "Benchmark iterations: "
                    << std::right << std::setw(10) << comppare::config::bench_iters() << "\n"
                    << std::left << comppare::internal::ansi::BOLD("=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*") << "\n\n";

                // Print header for the output table
                std::cout << comppare::internal::ansi::UNDERLINE << comppare::internal::ansi::BOLD
                          << std::left
                          << std::setw(PRINT_COL_WIDTH) << "Implementation"
                          << std::right
                          << std::setw(PRINT_COL_WIDTH) << "ROI µs/Iter"
                          << std::setw(PRINT_COL_WIDTH) << "Func µs"
                          << std::setw(PRINT_COL_WIDTH) << "Ovhd µs";

                // prints metric header
                auto &&_print_metric_header = [this]<std::size_t I>()
                {
                    for (std::size_t m = 0; m < this->template spec_metric_count<I>(); ++m)
                    {
                        std::cout << std::setw(PRINT_COL_WIDTH)
                                  << comppare::internal::ansi::UNDERLINE(
                                         comppare::internal::ansi::BOLD(
                                             std::string(this->template spec_metric_name<I>(m)) + "[" + std::to_string(I) + "]"));
                    }
                };

                // lambda to call print metric header across each metric by unpacking I
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    // .template is a disambiguator as
                    // if using _print_metric_header<I>(),
                    // the compiler will treat `<` as an operator
                    (_print_metric_header.template operator()<I>(), ...);
                }(std::make_index_sequence<NUM_OUT>{});

                std::cout << std::endl;
            }

            /**
             * @brief Compute the error metrics for each output specification.
             *
             *
             * @param errs The tuple of error policies.
             * @param test The test output.
             * @param ref The reference output.
             */
            void compute_errors(PolicyTup &errs, const OutTup &test, const OutTup &ref)
            {
                auto &&_compute_errors = [&]<std::size_t I>()
                { comppare::internal::policy::compute_error(std::get<I>(errs), std::get<I>(test), std::get<I>(ref)); };

                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (_compute_errors.template operator()<I>(), ...);
                }(std::make_index_sequence<NUM_OUT>{});
            }

            /**
             * @brief Check if any of the error policies indicate a failure.
             *
             * @param errs The tuple of error policies to check.
             * @return true if any policy indicates a failure, false otherwise.
             */
            bool any_fail(const PolicyTup &errs) const
            {
                auto &&_any_fail = [&]<std::size_t I>() -> bool
                {
                    return comppare::internal::policy::is_fail(std::get<I>(errs));
                };

                return [&]<std::size_t... I>(std::index_sequence<I...>) -> bool
                {
                    bool fail = false;
                    ((fail |= _any_fail.template operator()<I>()), ...);
                    return fail;
                }(std::make_index_sequence<NUM_OUT>{});
            }

            /**
             * @brief Print the metrics for each output specification.
             *
             * @param errs The tuple of error policies containing the metrics to print.
             */
            void print_metrics(const PolicyTup &errs) const
            {
                auto &&_print_metrics = [&errs]<std::size_t I>()
                {
                    for (std::size_t m = 0; m < spec_metric_count<I>(); ++m)
                        std::cout << std::setw(PRINT_COL_WIDTH) << std::scientific << std::get<I>(errs).metric(m);
                };

                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    (_print_metrics.template operator()<I>(), ...);
                }(std::make_index_sequence<NUM_OUT>{});
            }

            /**
             * @brief Get the output by index.
             *
             * @param idx The index of the output.
             * @return Shared pointer to the output tuple.
             */
            inline OutPtr get_output_by_index_(const size_t idx) const
            {
                if (outputs_.empty())
                    throw std::logic_error("run() has not been executed");
                if (idx >= outputs_.size())
                    throw std::out_of_range("Index out of range for outputs");

                return outputs_[idx];
            }

            /**
             * @brief Get the output by implementation name.
             *
             * @param name The name of the implementation.
             * @return Shared pointer to the output tuple.
             */
            inline OutPtr get_output_by_name_(const std::string_view name) const
            {
                if (outputs_.empty())
                    throw std::logic_error("run() has not been executed");
                for (size_t i = 0; i < impls_.size(); ++i)
                {
                    if (impls_[i].name == name)
                        return outputs_[i];
                }
                std::stringstream os;
                os << "Output with name '" << name << "' not found";
                throw std::invalid_argument(os.str());
            }

            /**
             * @brief Unpack the output tuple into the provided output pointers.
             *
             * @param outtup The output tuple to unpack.
             * @param outs The output pointers to fill.
             */
            void unpack_output_(const OutTup &outtup, outtype_t<OutputSpecs> *...outs) const
            {
                std::apply(
                    [&](auto &...outtup_elem)
                    {
                        ((*outs = outtup_elem), ...);
                    },
                    outtup);
            }

        public:
            /**
             * @brief Construct a new OutputContext
             *
             * @tparam Ins The types of the input arguments
             * @param ins The input arguments
             *
             * This constructor initializes the OutputContext with the provided input arguments.
             * The inputs are perfectly forwarded to allow for move and copy semantics.
             */
            template <typename... Ins>
            explicit OutputContext(Ins &&...ins)
                : inputs_(std::forward<Ins>(ins)...) {}

            /** @brief Deleted copy constructor */
            OutputContext(const OutputContext &other) = delete;
            /** @brief Deleted copy assignment operator */
            OutputContext &operator=(const OutputContext &other) = delete;
            /** @brief Deleted move constructor */
            OutputContext(OutputContext &&other) = delete;
            /** @brief Deleted move assignment operator */
            OutputContext &operator=(OutputContext &&other) = delete;

            /**
             * @brief Set a reference implementation
             *
             * @tparam F The type of the function
             * @param name The name of the implementation
             * @param f The function to execute
             * @return The implementation instance
             *
             * This function sets a reference implementation to the comparison framework.
             * The reference implementation is always the first one added and is used as the baseline for comparison.
             */
            template <typename F>
                requires std::invocable<F, const std::decay_t<Inputs> &..., outtype_t<OutputSpecs> &...>
            Impl &set_reference(std::string name, F &&f)
            {
                impls_.insert(impls_.begin(), {std::move(name), Func(std::forward<F>(f)), &inputs_, this});
                return impls_.front();
            }

            /**
             * @brief Add a new implementation to the comparison framework
             *
             * @tparam F The type of the function
             * @param name The name of the implementation
             * @param f The function to execute
             * @return The implementation instance
             *
             * This function adds a new implementation to the comparison framework.
             * The function will be run and compared against the reference implementation.
             */
            template <typename F>
                requires std::invocable<F, const std::decay_t<Inputs> &..., outtype_t<OutputSpecs> &...>
            Impl &add(std::string name, F &&f)
            {
                impls_.push_back({std::move(name), Func(std::forward<F>(f)), &inputs_, this});
                return impls_.back();
            }

            /*
            Getter for the output results
            */
            /**
             * @brief Get the reference output by pointer
             *
             * @return A shared pointer to the reference output tuple
             *
             * This function returns a shared pointer to the output of the reference implementation.
             */
            const OutPtr get_reference_output() const
            {
                return get_output_by_index_(0);
            }

            /**
             * @brief Get the output for a specific implementation by pointer
             *
             * @param idx The index of the implementation
             * @return A shared pointer to the output of the implementation
             *
             * This function returns a shared pointer to the output of the implementation at the specified index.
             */
            const OutPtr get_output(const size_t idx) const
            {
                return get_output_by_index_(idx);
            }

            /**
             * @brief Get the output for a specific implementation by name
             *
             * @param name The name of the implementation
             * @return A shared pointer to the output of the implementation
             *
             * This function returns a shared pointer to the output of the implementation with the specified name.
             */
            const OutPtr get_output(const std::string_view name) const
            {
                return get_output_by_name_(name);
            }

            /**
             * @brief Copies the reference output into provided pointer to variables.
             *
             *
             * @param outs One pointer per output element. Each pointer must point
             *             to writable storage of the corresponding output type.
             *
             * @details
             * Internally, this looks up the output tuple at index 0 (the reference),
             * and unpacks its elements into the provided pointers. This allows
             * callers to access values without dealing with `std::tuple` directly.
             */
            void get_reference_output(outtype_t<OutputSpecs> *...outs) const
                requires(sizeof...(OutputSpecs) > 0)
            {
                const auto &outtup = *get_output_by_index_(0);
                unpack_output_(outtup, outs...);
            }

            /**
             * @brief Copies the outputs of a specific implementation by index into provided pointer to variables.
             *
             *
             * @param outs One pointer per output element. Each pointer must point
             *             to writable storage of the corresponding output type.
             *
             * @details
             * Internally, this looks up the output tuple at the specified index,
             * and unpacks its elements into the provided pointers. This allows
             * callers to access values without dealing with `std::tuple` directly.
             */
            void get_output(const size_t idx, outtype_t<OutputSpecs> *...outs) const
                requires(sizeof...(OutputSpecs) > 0)
            {
                const auto &outtup = *get_output_by_index_(idx);
                unpack_output_(outtup, outs...);
            }

            /**
             * @brief Copies the outputs of a specific implementation by name into provided pointer to variables.
             *
             *
             * @param outs One pointer per output element. Each pointer must point
             *             to writable storage of the corresponding output type.
             *
             * @details
             * Internally, this looks up the output tuple at the specified name,
             * and unpacks its elements into the provided pointers. This allows
             * callers to access values without dealing with `std::tuple` directly.
             */
            void get_output(const std::string_view name, outtype_t<OutputSpecs> *...outs) const
                requires(sizeof...(OutputSpecs) > 0)
            {
                const auto &outtup = *get_output_by_name_(name);
                unpack_output_(outtup, outs...);
            }

            /**
             * @brief Runs the comparison for all added implementations.
             *
             * @param argc Number of command line arguments
             * @param argv Array of command line arguments
             */
            void run(int argc = 0,
                     char **argv = nullptr)
            {
                comppare::internal::helper::parse_args(argc, argv);

                if (impls_.empty())
                {
                    std::cerr << "\n*----------*\nNo implementations added to the ComPPare Framework.\n*----------*\n";
                    return;
                }

                outputs_.reserve(impls_.size()); // reserve space for outputs -- resize and use index works too.

                print_header();

                // Main loop to iterate over all implementations
                for (size_t k = 0; k < impls_.size(); ++k)
                {
                    // Get the current implementation
                    auto &impl = impls_[k];

                    OutTup outs;

                    double func_duration;
                    /*
                    use std::apply to unpack the inputs and outputs completely to do 1 function call of the implementation
                    this is equivalent to calling:
                    impl.fn(inputs[0], inputs[1], ..., outputs[0], outputs[1], iters, roi_us);
                    */
                    std::apply([&](auto const &...in)
                               { std::apply(
                                     [&](auto &...out)
                                     {
                                         auto func_start = comppare::config::clock_t::now();
                                         impl.fn(in..., out...);
                                         auto func_end = comppare::config::clock_t::now();
                                         func_duration = std::chrono::duration<double, std::micro>(func_end - func_start).count();
                                     },
                                     outs); },
                               inputs_);

                    // Calculate the time taken by the function in microseconds
                    double roi_us = comppare::config::get_roi_us();
                    double warmup_us = comppare::config::get_warmup_us();
                    double func_us = func_duration - warmup_us;
                    double ovhd_us = func_us - roi_us;

                    roi_us /= static_cast<double>(comppare::config::bench_iters());

                    PolicyTup errs{};
                    if (k)
                    {
                        compute_errors(errs, outs, *outputs_[0]);
                    }
                    outputs_.push_back(std::make_shared<OutTup>(std::move(outs)));
                    // print row
                    std::cout << comppare::internal::ansi::RESET
                              << std::left << std::setw(PRINT_COL_WIDTH) << comppare::internal::ansi::GREEN(impl.name)
                              << std::fixed << std::setprecision(2) << std::right
                              << comppare::internal::ansi::YELLOW
                              << std::setw(PRINT_COL_WIDTH) << roi_us
                              << comppare::internal::ansi::DIM
                              << std::setw(PRINT_COL_WIDTH) << func_us
                              << std::setw(PRINT_COL_WIDTH) << ovhd_us
                              << comppare::internal::ansi::RESET;

                    print_metrics(errs);
                    if (k && any_fail(errs))
                        std::cout << comppare::internal::ansi::BG_RED("<-- FAIL");
                    std::cout << '\n';

                } /* for impls */

                comppare::current_state::set_using_plugin(true);
                if (plugin_)
                {
                    plugin_->initialize(argc, argv);
                    plugin_->run();
                }
                comppare::current_state::set_using_plugin(false);
            } /* run */
        }; /* OutputContext */
    }; /* InputContext */

    /**
     * @brief Helper function to create a comppare object.
     *
     * This function simplifies the creation of a comppare object by deducing the input types.
     * It takes input arguments and returns an `OutputContext` object that can be used to add implementations and run comparisons.
     * The output types must be specified explicitly as template parameters, while the input types are deduced from the function arguments.
     * The function arguments are perfectly forwarded to instantiate the `OutputContext` object.
     *
     * @tparam Outputs The types of the output specifications.
     * @tparam Inputs The types of the input specifications -- deduced from function arguments.
     * @param ins The input arguments.
     * @return typename InputContext<Inputs...>::OutputContext<Outputs...>
     */
    template <typename... Outputs, typename... Inputs>
    auto make_comppare(Inputs &&...ins)
    {
        return typename InputContext<std::decay_t<Inputs>...>::template OutputContext<std::decay_t<Outputs>...>(
            std::forward<Inputs>(ins)...);
    }
} // namespace comppare

/**
 * @brief Macro to mark the start of a hot loop for benchmarking.
 * This macro defines a lambda function `hotloop_body` that encapsulates the code to be benchmarked.
 */
#define HOTLOOPSTART \
    auto &&hotloop_body = [&]() { /* start of lambda */

/**
 * @brief Internal macro to perform the warm-up and timed benchmarking loops.
 * This macro is used within the `HOTLOOPEND` macro to execute the benchmarking process.
 */
#define COMPPARE_HOTLOOP_BENCH                                         \
    /* Warm-up */                                                      \
    auto warmup_t0 = comppare::config::clock_t::now();                 \
    for (std::size_t i = 0; i < comppare::config::warmup_iters(); ++i) \
        hotloop_body();                                                \
    auto warmup_t1 = comppare::config::clock_t::now();                 \
    comppare::config::set_warmup_us(warmup_t0, warmup_t1);             \
                                                                       \
    /* Timed */                                                        \
    comppare::config::reset_roi_us();                                  \
    auto t0 = comppare::config::clock_t::now();                        \
    for (std::size_t i = 0; i < comppare::config::bench_iters(); ++i)  \
        hotloop_body();                                                \
    auto t1 = comppare::config::clock_t::now();                        \
                                                                       \
    if (comppare::config::get_roi_us() == double(0.0))                 \
        comppare::config::set_roi_us(t0, t1);

#ifdef PLUGIN_HOTLOOP_BENCH
#define HOTLOOPEND                               \
    }                                            \
    ; /* end lambda */                           \
                                                 \
    if (comppare::current_state::using_plugin()) \
    {                                            \
        PLUGIN_HOTLOOP_BENCH;                    \
    }                                            \
    else                                         \
    {                                            \
        COMPPARE_HOTLOOP_BENCH;                  \
    }
#else
#define HOTLOOPEND     \
    }                  \
    ; /* end lambda */ \
                       \
    COMPPARE_HOTLOOP_BENCH;
#endif

/**
 * @brief Macro to wrap a code block for benchmarking.
 */
#define HOTLOOP(LOOP_BODY) \
    HOTLOOPSTART LOOP_BODY HOTLOOPEND

/**
 * @brief Macro to mark the start of a manual timer for benchmarking.
 */
#define MANUAL_TIMER_START \
    auto t_manual_start = comppare::config::clock_t::now();

/**
 * @brief Macro to mark the end of a manual timer for benchmarking.
 */
#define MANUAL_TIMER_END                                   \
    auto t_manual_stop = comppare::config::clock_t::now(); \
    SET_ITERATION_TIME(t_manual_stop - t_manual_start);

#ifdef PLUGIN_HOTLOOP_BENCH
#define SET_ITERATION_TIME(TIME)                  \
    if (comppare::current_state::using_plugin())  \
    {                                             \
        PLUGIN_SET_ITERATION_TIME(TIME);          \
    }                                             \
    else                                          \
    {                                             \
        comppare::config::increment_roi_us(TIME); \
    }
#else
#define SET_ITERATION_TIME(TIME) \
    comppare::config::increment_roi_us(TIME);
#endif

#if defined(__CUDACC__)
#define GPU_PREFIX cuda
#elif defined(__HIPCC__)
#define GPU_PREFIX hip
#endif

#if defined(GPU_PREFIX)

#if defined(HAVE_GOOGLE_BENCHMARK)
#warning "Not Recommended to use Google Benchmark with GPU_HOTLOOPEND macro. Use SET_ITERATION_TIME and manual timing instead."
#endif

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

/**
 * @brief Macro to mark the start of a GPU hot loop for benchmarking.
 * This macro defines a lambda function `hotloop_body` that encapsulates the code to be benchmarked.
 */
#define GPU_HOTLOOPSTART \
    auto &&hotloop_body = [&]() { /* start of lambda */

/**
 * @brief Internal macro to perform the warm-up and timed benchmarking loops.
 * This macro is used within the `GPU_HOTLOOPEND` macro to execute the benchmarking process.
 */
#define GPU_COMPPARE_HOTLOOP_BENCH                                     \
    /* Warm-up */                                                      \
    CONCAT(GPU_PREFIX, Event_t)                                        \
    start_, stop_;                                                     \
    CONCAT(GPU_PREFIX, EventCreate)(&start_);                          \
    CONCAT(GPU_PREFIX, EventCreate)(&stop_);                           \
    CONCAT(GPU_PREFIX, EventRecord)(start_);                           \
    for (std::size_t i = 0; i < comppare::config::warmup_iters(); ++i) \
        hotloop_body();                                                \
    CONCAT(GPU_PREFIX, EventRecord)(stop_);                            \
    CONCAT(GPU_PREFIX, EventSynchronize)(stop_);                       \
    float ms_warmup_;                                                  \
    CONCAT(GPU_PREFIX, EventElapsedTime)(&ms_warmup_, start_, stop_);  \
    comppare::config::set_warmup_us(1e3 * ms_warmup_);                 \
                                                                       \
    /* Timed */                                                        \
    comppare::config::reset_roi_us();                                  \
    CONCAT(GPU_PREFIX, EventRecord)(start_);                           \
    for (std::size_t i = 0; i < comppare::config::bench_iters(); ++i)  \
        hotloop_body();                                                \
    CONCAT(GPU_PREFIX, EventRecord)(stop_);                            \
    CONCAT(GPU_PREFIX, EventSynchronize)(stop_);                       \
    float ms_;                                                         \
    CONCAT(GPU_PREFIX, EventElapsedTime)(&ms_, start_, stop_);         \
    if (comppare::config::get_roi_us() == double(0.0))                 \
        comppare::config::set_roi_us(1e3 * ms_);                       \
    CONCAT(GPU_PREFIX, EventDestroy)(start_);                          \
    CONCAT(GPU_PREFIX, EventDestroy)(stop_);

#if defined(GPU_PLUGIN_HOTLOOP_BENCH)

#define GPU_HOTLOOPEND                           \
    }                                            \
    ; /* end lambda */                           \
    if (comppare::current_state::using_plugin()) \
    {                                            \
        GPU_PLUGIN_HOTLOOP_BENCH;                \
    }                                            \
    else                                         \
    {                                            \
        GPU_COMPPARE_HOTLOOP_BENCH;              \
    }
#else

#define GPU_HOTLOOPEND \
    }                  \
    ; /* end lambda */ \
    GPU_COMPPARE_HOTLOOP_BENCH;

#endif

/**
 * @brief Macro to start a manual timer for benchmarking.
 * This macro initializes GPU events and records the start time.
 */
#define GPU_MANUAL_TIMER_START                            \
    CONCAT(GPU_PREFIX, Event_t)                           \
    start_manual_timer, stop_manual_timer;                \
    CONCAT(GPU_PREFIX, EventCreate)(&start_manual_timer); \
    CONCAT(GPU_PREFIX, EventCreate)(&stop_manual_timer);  \
    CONCAT(GPU_PREFIX, EventRecord)(start_manual_timer);

/**
 * @brief Macro to stop a manual timer for benchmarking.
 * This macro records the stop time and synchronizes the GPU events.
 */
#define GPU_MANUAL_TIMER_END                                                                 \
    CONCAT(GPU_PREFIX, EventRecord)(stop_manual_timer);                                      \
    CONCAT(GPU_PREFIX, EventSynchronize)(stop_manual_timer);                                 \
    float ms_manual;                                                                         \
    CONCAT(GPU_PREFIX, EventElapsedTime)(&ms_manual, start_manual_timer, stop_manual_timer); \
    SET_ITERATION_TIME(1e3 * ms_manual);                                                     \
    CONCAT(GPU_PREFIX, EventDestroy)(start_manual_timer);                                    \
    CONCAT(GPU_PREFIX, EventDestroy)(stop_manual_timer);

#endif