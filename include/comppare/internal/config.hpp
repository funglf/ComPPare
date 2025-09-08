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
 * @file config.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file contains configuration settings for the ComPPare library.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 */

#pragma once
#include <chrono>
#include <utility>
#include <string_view>

namespace comppare
{
    /**
     * @brief Configuration singleton for the ComPPare library.
     * 
     * @note Singleton is chosen as this is a header only library, 
     * using global variables will require user to declare them in a .cpp file.
     */
    class config
    {
    public:
        /** @brief Type alias for the clock used in timing operations. */
        using clock_t = std::chrono::steady_clock;

        /** @brief Type alias for the time point used in timing operations. */
        using time_point_t = std::chrono::time_point<clock_t>;

        /** @brief Deleted copy constructor. */
        config(const config &) = delete;
        /** @brief Deleted move constructor. */
        config(config &&) = delete;
        /** @brief Deleted copy assignment operator. */
        config &operator=(const config &) = delete;
        /** @brief Deleted move assignment operator. */
        config &operator=(config &&) = delete;

        /** @brief Get the number of warmup iterations. */
        static uint64_t warmup_iters() { return instance().warmup_iters_; }
        /** @brief Set the number of warmup iterations. */
        static void set_warmup_iters(uint64_t v) { instance().warmup_iters_ = v; }

        /** @brief Get the number of benchmark iterations. */
        static uint64_t bench_iters() { return instance().bench_iters_; }
        /** @brief Set the number of benchmark iterations. */
        static void set_bench_iters(uint64_t v) { instance().bench_iters_ = v; }

        /** @brief Reset the ROI time to zero. */
        static void reset_roi_us() { instance().roi_ = double(0.0); }

        /**
         * @brief Set the roi us value using time points.
         *
         * @param start
         * @param end
         */
        static void set_roi_us(const time_point_t &start, const time_point_t &end) { instance().roi_ = std::chrono::duration<double, std::micro>(end - start).count(); }
        /** @brief Set the roi us value using float. */
        static void set_roi_us(const float start, const float end) { instance().roi_ = static_cast<double>(end - start); }
        /** @brief Set the roi us value using double. */
        static void set_roi_us(const double start, const double end) { instance().roi_ = end - start; }

        /**
         * @brief Set the roi us value using a `std::chrono::duration`.
         *
         * @tparam Rep
         * @tparam Period
         * @param v
         */
        template <typename Rep, typename Period>
        static void set_roi_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().roi_ = micros;
        }
        /** @brief Set the roi us value using double. */
        static void set_roi_us(const double v) { instance().roi_ = v; }
        /** @brief Set the roi us value using float. */
        static void set_roi_us(const float v) { instance().roi_ = static_cast<double>(v); }

        /**
         * @brief Increment the roi us value using time points.
         *
         * @param start
         * @param end
         */
        template <typename Rep, typename Period>
        static void increment_roi_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().roi_ += micros;
        }
        /** @brief Increment the roi us value using double. */
        static void increment_roi_us(const double v) { instance().roi_ += v; }
        /** @brief Increment the roi us value using float. */
        static void increment_roi_us(const float v) { instance().roi_ += static_cast<double>(v); }

        /**
         * @brief Set the warmup us value using time points.
         * @param start
         * @param end
         */
        static void set_warmup_us(const time_point_t &start, const time_point_t &end) { instance().warmup_ = std::chrono::duration<double, std::micro>(end - start).count(); }
        /**
         * @brief Set the warmup us value using float.
         * @param start
         * @param end
         */
        static void set_warmup_us(const float start, const float end) { instance().warmup_ = static_cast<double>(end - start); }
        /**
         * @brief Set the warmup us value using double.
         * @param start
         * @param end
         */
        static void set_warmup_us(const double start, const double end) { instance().warmup_ = end - start; }

        /**
         * @brief Set the warmup us value using a `std::chrono::duration`.
         *
         * @tparam Rep
         * @tparam Period
         * @param v
         */
        template <typename Rep, typename Period>
        static void set_warmup_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().warmup_ = micros;
        }
        /** @brief Set the warmup us value using double. */
        static void set_warmup_us(const double v) { instance().warmup_ = v; }
        /** @brief Set the warmup us value using float. */
        static void set_warmup_us(const float v) { instance().warmup_ = static_cast<double>(v); }

        /** @brief Get the current roi us value. */
        static double get_roi_us() { return instance().roi_; }
        /** @brief Get the current warmup us value. */
        static double get_warmup_us() { return instance().warmup_; }

        /**
         * @brief Get the floating-point tolerance for a specific floating-point type.
         *
         * @tparam T
         * @return T&
         */
        template <std::floating_point T = double>
        static T &fp_tolerance()
        {
            static T tol = std::numeric_limits<T>::epsilon() * 1e3; // Default tolerance
            return tol;
        }

        /**
         * @brief Set the floating-point tolerance for a specific floating-point type.
         *
         * @tparam T
         * @param v The tolerance value to set.
         */
        template <std::floating_point T = double>
        static void set_fp_tolerance(T v)
        {
            fp_tolerance<T>() = v;
        }

        /**
         * @brief Set the floating-point tolerance for all supported types.
         *
         * @param v The tolerance value to set for all floating-point types.
         */
        static void set_all_fp_tolerance(long double v)
        {
            fp_tolerance<float>() = static_cast<float>(v);
            fp_tolerance<double>() = static_cast<double>(v);
            fp_tolerance<long double>() = v;
        }

    private:
        /**
         * @brief Construct a new config object
         *
         * @note This constructor is private to enforce the singleton pattern.
         */
        config() = default;

        /** @brief Get the singleton instance of the config class. */
        static config &instance()
        {
            static config inst;
            return inst;
        }

        /** @brief The current roi us value. */
        double roi_{0.0};
        /** @brief The current warmup us value. */
        double warmup_{0.0};

        /** @brief The number of warmup iterations. */
        uint64_t warmup_iters_{100};
        /** @brief The number of benchmark iterations. */
        uint64_t bench_iters_{100};
    };

    /*
    Singleton Class for the current state
    */

    /**
     * @brief Singleton class for the current state.
     *
     * This singleton class holds the current state of the benchmarking process.
     * It tracks whether a plugin is being used and the name of the current implementation.
     * @note Singleton is chosen as this is a header only library, 
     * using global variables will require user to declare them in a .cpp file.
     */
    class current_state
    {
    public:
        /** @brief Deleted copy constructor. */
        current_state(const current_state &) = delete;
        /** @brief Deleted move constructor. */
        current_state(current_state &&) = delete;
        /** @brief Deleted copy assignment operator. */
        current_state &operator=(const current_state &) = delete;
        /** @brief Deleted move assignment operator. */
        current_state &operator=(current_state &&) = delete;

        /** @brief Get if a plugin is being used currently. */
        static bool using_plugin() { return instance().using_plugin_; }
        /** @brief Set if a plugin is being used currently. */
        static void set_using_plugin(bool v) { instance().using_plugin_ = v; }

        /** @brief Get the name of the current implementation. */
        static std::string_view impl_name() { return instance().impl_name_; }
        /** @brief Set the name of the current implementation. */
        static void set_impl_name(std::string_view name) { instance().impl_name_ = name; }

    private:
        /** @brief Private Default kconstructor. */
        current_state() = default;

        /** @brief Get the singleton instance of the current_state class. */
        static current_state &instance()
        {
            static current_state inst;
            return inst;
        }

        /** @brief The current state of plugin usage. */
        bool using_plugin_{false};
        /** @brief The name of the current implementation. */
        std::string_view impl_name_;
    };

}
