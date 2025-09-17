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
 * @file policy.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file contains error policies for comparing values in the ComPPare library.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 */
#pragma once
#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <limits>
#include <ostream>
#include <sstream>
#include <variant>
#include <stdexcept>

#include <comppare/internal/concepts.hpp>
#include <comppare/internal/ansi.hpp>

namespace comppare::internal::policy
{

    /**
     * @brief Wrapper for a value that can be streamed to an output stream.
     *
     * @tparam T The type of the value to be wrapped. Must satisfy
     *           `Streamable` (i.e. it can be inserted into a `std::ostream`).
     *
     * Implementation Details:
     * - Private Data Members:
     *   1. `value_`   – the value of the metric
     *   2. `is_fail_` – indicates if the metric has failed
     *   3. `valid_`   – indicates if the metric is valid. (eg. invalid if size mismatch between 2 vectors)
     *   4. `err_msg_` – is an error message if the metric is invalid. (eg. outputs "size mismatch" if the size of 2 vectors is different)
     *
     * - `operator<<`:
     *   1. Copies the current formatting state of the stream with copyfmt
     *   2. Streams the value `value_` into a temporary `std::ostringstream`
     *   3. If the metric is valid and not failed, writes the value to the original stream.
     *      If the metric is valid but failed, writes the value in red color to the original stream.
     *      If the metric is invalid, writes the error message in red color to the original stream.
     *   4. Restores the saved formatting state
     */
    template <comppare::internal::concepts::Streamable T>
    class MetricValue
    {
        T value_;
        bool is_fail_{false};

        bool valid_{true};
        std::string_view err_msg_;

    public:
        MetricValue(T v) : value_(v), err_msg_(""), valid_(true), is_fail_(false) {}
        MetricValue(T v, bool is_fail) : value_(v), is_fail_(is_fail), valid_(true), err_msg_("") {}
        MetricValue(T v, bool is_fail, bool valid, std::string_view msg) : value_(v), is_fail_(is_fail), valid_(valid), err_msg_(msg) {}

        /**
         * @brief Overloaded operator<< to stream the value or error message
         *
         * @param os The output stream
         * @param mv The MetricValue to stream
         * @return The modified output stream
         *
         * Output Behavior:
         *  - If the metric is valid and not failed, writes the value to the original stream.
         *  - If the metric is valid but failed, writes the value in red color to the original stream.
         *  - If the metric is invalid, writes the error message in red color to the original stream.
         */
        friend std::ostream &
        operator<<(std::ostream &os, MetricValue<T> const &mv)
        {
            std::ios saved(nullptr);
            saved.copyfmt(os);

            std::ostringstream tmp;
            tmp.copyfmt(os);
            if (mv.valid_ && !mv.is_fail_)
                tmp << mv.value_;
            else if (mv.valid_ && mv.is_fail_)
                tmp << comppare::internal::ansi::RED(mv.value_);
            else
                tmp << comppare::internal::ansi::RED(mv.err_msg_);
            std::string body = std::move(tmp).str();

            os.width(0);
            os << body;

            os.copyfmt(saved);
            return os;
        }
    };

    /**
     * @brief Partial specialisation for not a MetricValue
     */
    template <typename>
    struct is_metric_value : std::false_type
    {
    };

    /**
     * @brief Partial specialisation for any MetricValue<U> and returns true.
     */
    template <typename U>
    struct is_metric_value<MetricValue<U>> : std::true_type
    {
    };

    /**
     * @brief Checks if a type is a MetricValue
     *
     * @tparam M The type to check
     *
     * If M is of type MetricValue<T> for some T, then is_metric_value_v<M> is true.
     * Otherwise, it is false.
     * This is implemented using template specialization of the is_metric_value struct.
     */
    template <typename M>
    inline constexpr bool is_metric_value_v = is_metric_value<std::remove_cvref_t<M>>::value;

    /**
     * @brief Concept for a MetricValue
     *
     * @tparam M The type to check
     */
    template <typename M>
    concept IsMetricValue = is_metric_value_v<M>;

    /*
    Concept for a valid Error Policy

    It requires:
    - metric_count() to return the number of metrics
    - metric_name(std::size_t i) to return the name of the metric at index i
    - compute_error(const Val &a, const Val &b, double tol) to compute the error
      between two values a and b with a given tolerance tol -- or not
    - metric(std::size_t) to return the value of the metric as MetricValue<T>
    - is_fail() to return true if the error exceeds the tolerance
    */

    /**
     * @brief Concept for a valid Error Policy
     *
     * @tparam Val The type of the values being compared
     * @tparam EP The type of the error policy
     *
     * Requirements:
     * - Static Members:
     *   1. `metric_count()` - returns the number of metrics
     *   2. `metric_name(std::size_t i)` - returns the name of the metric at index i
     * - Member Functions:
     *   1. `compute_error(const Val &a, const Val &b)`
     *      - computes the error between two values a and b with or without a given tolerance (via global config)
     *   2. `metric(std::size_t)`
     *      - returns the value of the metric as MetricValue<T>,
     *      - or convertible to double,
     *      - or convertible to std::string
     *   3. `is_fail()`
     *      - returns true if the case passed, any custom condition is allowed.
     */
    template <typename Val, typename EP>
    concept ErrorPolicy = requires
    // static members
    {
        { EP::metric_count() } -> std::convertible_to<std::size_t>;
        { EP::metric_name(std::size_t{}) } -> std::convertible_to<std::string_view>;
    } &&
                          // compute error -- either with or without tolerance
                          (requires(EP ep, const Val &a, const Val &b) { ep.compute_error(a, b); }) &&
                          // metric() returns value of the metric
                          (requires(EP ep, std::size_t i) {
            { ep.metric(i) } -> IsMetricValue; } || requires(EP ep, std::size_t i) {
            { ep.metric(i) } -> std::convertible_to<double>; } || requires(EP ep, std::size_t i) {
            { ep.metric(i) } -> std::same_as<std::string>; }) &&
                          // is_fail() -- either with or without tolerance
                          (requires(EP ep) {
            { ep.is_fail() } -> std::convertible_to<bool>; });

    /**
     * @brief Wrapper function to compute the error using the given instance of the error policy.
     *
     * @tparam EP Error Policy type
     * @tparam V Value type
     * @param ep Error policy instance
     * @param a First value
     * @param b Second value
     */
    template <class EP, class V>
    inline void compute_error(EP &ep, const V &a, const V &b)
    {
        ep.compute_error(a, b);
    }

    /**
     * @brief Wrapper function to check if the error policy indicates a failure.
     *
     * @tparam EP Error Policy type
     * @param ep Error policy instance
     * @return true if the error policy indicates a failure, false otherwise
     */
    template <class EP>
    inline bool is_fail(const EP &ep)
    {
        return ep.is_fail();
    }

    /**
     * @namespace Namespace for automatic error policy selection.
     * This namespace contains error policies that can be automatically selected based on the type of the output value.
     */
    namespace autopolicy
    {
        /**
         * @brief Concept for types supported by automatic error policy selection.
         *
         * @tparam T Value type
         *
         * Supported types include:
         * - Arithmetic types (e.g., int, float, double)
         * - std::string
         * - Ranges of arithmetic types (e.g., std::vector<int>, std::deque<float>)
         */
        template <typename T>
        concept SupportedByAutoPolicy =
            comppare::internal::concepts::Arithmetic<T> ||
            comppare::internal::concepts::String<T> ||
            comppare::internal::concepts::RangeOfArithmetic<T>;

        /*
        Error Policy for scalar/numbers
        */

        /**
         * @brief Concept for types supported by automatic error policy selection.
         *
         * Error policy for arithmetic types (e.g., int, float, double).
         * Computes the absolute error between two values and checks if it exceeds a configurable tolerance.
         *
         * @tparam T Value type
         */
        template <typename T>
            requires comppare::internal::concepts::Arithmetic<T>
        class ArithmeticErrorPolicy
        {
            T error_ = T(0);
            std::string err_msg_;
            bool fail_{false};
            bool valid_{true};
            T tolerance_;

            static constexpr std::array names{"Total|err|"};

        public:
            ArithmeticErrorPolicy()
            {
                if constexpr (!std::is_floating_point_v<T>)
                {
                    // cast tolerance to T for integral types
                    tolerance_ = static_cast<T>(comppare::config::fp_tolerance<double>());
                }
                else
                {
                    // use tolerance as is for floating-point types
                    tolerance_ = comppare::config::fp_tolerance<T>();
                }
            }

            ~ArithmeticErrorPolicy() = default;

            static constexpr std::size_t metric_count() { return 1; }
            static constexpr std::string_view metric_name(std::size_t) { return names[0]; }

            MetricValue<T> metric(std::size_t) const
            {
                return MetricValue<T>(error_, is_fail(), valid_, err_msg_);
            }

            bool is_fail() const { return fail_; }

            void compute_error(const T &a, const T &b)
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    if (!std::isfinite(a) || !std::isfinite(b))
                    {
                        error_ = std::numeric_limits<T>::quiet_NaN();
                        err_msg_ = "NAN/INF";
                        valid_ = false;
                        return;
                    }
                }

                T e = std::abs(a - b);

                if (e > tolerance_)
                    fail_ = true;

                error_ = e;
            }
        };

        /**
         * @brief Error policy for std::string.
         * Compares two strings for equality.
         */
        class StringEqualPolicy
        {
            bool eq_{true};

            static constexpr std::array names{"Equal?"};

        public:
            StringEqualPolicy() = default;
            ~StringEqualPolicy() = default;

            static constexpr std::size_t metric_count() { return 1; }
            static constexpr std::string_view metric_name(std::size_t) { return names[0]; }

            MetricValue<std::string> metric(std::size_t) const
            {
                return MetricValue<std::string>(eq_ ? "true" : "false", is_fail());
            }

            bool is_fail() const { return !eq_; }

            void compute_error(const std::string &a, const std::string &b) { eq_ = (a == b); }
        };

        /**
         * @brief Error policy for ranges of arithmetic types.
         * 
         * Compares two ranges element-wise and computes metrics such as maximum error, mean error, and total error.
         * 
         * @tparam R Range type (e.g., std::vector<int>, std::deque<float>)
         */
        template <typename R>
            requires comppare::internal::concepts::RangeOfArithmetic<R>
        class RangeErrorPolicy
        {
            using T = std::remove_cvref_t<std::ranges::range_value_t<R>>;

            T max_error_ = T(0);
            T total_error_ = T(0);
            std::size_t elem_cnt_ = 0;

            bool fail_{false};  /** < @brief Indicates if the current error state is a failure. */
            bool valid_{true};  /** < @brief Indicates if the current error state is valid. */
            std::string err_msg_;

            T tolerance_;

            static constexpr std::array names{"Max|err|", "Mean|err|", "Total|err|"};

            MetricValue<T> get_max() const
            {
                return MetricValue<T>(max_error_, is_fail(), valid_, err_msg_);
            }

            MetricValue<T> get_mean() const
            {
                if (elem_cnt_ && valid_)
                    return MetricValue<T>(total_error_ / static_cast<T>(elem_cnt_), is_fail(), valid_, err_msg_);
                else
                    return MetricValue<T>(T(0), is_fail(), valid_, err_msg_);
            }

            MetricValue<T> get_total() const
            {
                return MetricValue<T>(total_error_, is_fail(), valid_, err_msg_);
            }

        public:
            RangeErrorPolicy()
            {
                if constexpr (!std::is_floating_point_v<T>)
                {
                    // cast tolerance to T for integral types
                    tolerance_ = static_cast<T>(comppare::config::fp_tolerance<double>());
                }
                else
                {
                    // use tolerance as is for floating-point types
                    tolerance_ = comppare::config::fp_tolerance<T>();
                }
            }

            ~RangeErrorPolicy() = default;

            static constexpr std::size_t metric_count() { return names.size(); }
            static constexpr std::string_view metric_name(std::size_t i) { return names[i]; }

            MetricValue<T> metric(std::size_t i) const
            {
                switch (i)
                {
                case 0:
                    return get_max();
                case 1:
                    return get_mean();
                case 2:
                    return get_total();
                default:
                    throw std::out_of_range("Invalid metric index");
                }
            }

            /**
             * @brief Checks if the current error state is a failure.
             * 
             * @return true if the error state is a failure, false otherwise.
             * if the error state is invalid, it returns false.
             */
            bool is_fail() const
            {
                if (valid_)
                    return fail_;
                else
                    return false;
            }

            void compute_error(const R &a, const R &b)
            {
                if (std::ranges::size(a) != std::ranges::size(b))
                {
                    // invalid if sizes are different
                    valid_ = false; 
                    err_msg_ = "Size mismatch";
                    elem_cnt_ = 0;
                    return;
                }

                auto ia = std::ranges::begin(a);
                auto ib = std::ranges::begin(b);
                for (; ia != std::ranges::end(a) && ib != std::ranges::end(b); ++ia, ++ib)
                {
                    if constexpr (std::is_floating_point_v<T>)
                    {
                        if (!std::isfinite(*ia) || !std::isfinite(*ib))
                        {
                            max_error_ = std::numeric_limits<T>::quiet_NaN();
                            total_error_ = std::numeric_limits<T>::quiet_NaN();
                            elem_cnt_ = 0;

                            // invalid if any element is NAN/INF
                            valid_ = false;
                            err_msg_ = "NAN/INF";
                            return;
                        }
                    }

                    T e = std::abs(*ia - *ib);

                    if (e > tolerance_)
                    {
                        fail_ = true;
                        elem_cnt_++;
                    }

                    total_error_ += e;
                    max_error_ = std::max(max_error_, e);
                }
            }
        };


        /**
         * @brief AutoPolicy is a empty struct with alias type to deduce the appropriate error policy
         * @tparam T Type to deduce the error policy for
         */
        template <typename T>
        struct AutoPolicy;

        /**
         * @brief Partial specialization for types supported by automatic error policy selection.
         * 
         * @tparam T 
         */
        template <typename T>
            requires comppare::internal::concepts::Arithmetic<T>
        struct AutoPolicy<T>
        {
            using type = ArithmeticErrorPolicy<std::remove_cvref_t<T>>;
        };

        /**
         * @brief Partial specialization for string types.
         * 
         * @tparam T 
         */
        template <typename T>
            requires comppare::internal::concepts::String<T>
        struct AutoPolicy<T>
        {
            using type = StringEqualPolicy;
        };

        /**
         * @brief Partial specialization for ranges of arithmetic types.
         * 
         * @tparam T 
         */
        template <typename T>
            requires comppare::internal::concepts::RangeOfArithmetic<T>
        struct AutoPolicy<T>
        {
            using type = RangeErrorPolicy<T>;
        };

        /**
         * @brief Helper alias to get the automatic error policy type for a given type T.
         *
         * @tparam T The type to get the automatic error policy for.
         */
        template <typename T>
        using AutoPolicy_t = typename AutoPolicy<T>::type;
    }
}
