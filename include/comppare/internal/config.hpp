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
#include <chrono>
#include <utility>
#include <string_view>

namespace comppare
{
    /*
    Singleton Class for configuration settings
    Could be global struct -- currently using singleton to ensure.
    */
    class config
    {
    public:
        using clock_t = std::chrono::steady_clock;
        using time_point_t = std::chrono::time_point<clock_t>;

        config(const config &) = delete;
        config(config &&) = delete;
        config &operator=(const config &) = delete;
        config &operator=(config &&) = delete;

        static uint64_t warmup_iters() { return instance().warmup_iters_; }
        static void set_warmup_iters(uint64_t v) { instance().warmup_iters_ = v; }

        static uint64_t bench_iters() { return instance().bench_iters_; }
        static void set_bench_iters(uint64_t v) { instance().bench_iters_ = v; }

        static void reset_roi_us() { instance().roi_ = double(0.0); }

        static void set_roi_us(const time_point_t &start, const time_point_t &end) { instance().roi_ = std::chrono::duration<double, std::micro>(end - start).count(); }
        static void set_roi_us(const float start, const float end) { instance().roi_ = static_cast<double>(end - start); }
        static void set_roi_us(const double start, const double end) { instance().roi_ = end - start; }

        template <typename Rep, typename Period>
        static void set_roi_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().roi_ = micros;
        }
        static void set_roi_us(const double v) { instance().roi_ = v; }
        static void set_roi_us(const float v) { instance().roi_ = static_cast<double>(v); }

        template <typename Rep, typename Period>
        static void increment_roi_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().roi_ += micros;
        }
        static void increment_roi_us(const double v) { instance().roi_ += v; }
        static void increment_roi_us(const float v) { instance().roi_ += static_cast<double>(v); }

        static void set_warmup_us(const time_point_t &start, const time_point_t &end) { instance().warmup_ = std::chrono::duration<double, std::micro>(end - start).count(); }
        static void set_warmup_us(const float start, const float end) { instance().warmup_ = static_cast<double>(end - start); }
        static void set_warmup_us(const double start, const double end) { instance().warmup_ = end - start; }

        template <typename Rep, typename Period>
        static void set_warmup_us(std::chrono::duration<Rep, Period> v)
        {
            double micros = std::chrono::duration<double, std::micro>(v).count();
            instance().warmup_ = micros;
        }
        static void set_warmup_us(const double v) { instance().warmup_ = v; }
        static void set_warmup_us(const float v) { instance().warmup_ = static_cast<double>(v); }

        static double get_roi_us() { return instance().roi_; }
        static double get_warmup_us() { return instance().warmup_; }

        template <std::floating_point T>
        static T &fp_tolerance()
        {
            static T tol = std::numeric_limits<T>::epsilon() * 1e3; // Default tolerance
            return tol;
        }

        template <std::floating_point T>
        static void set_fp_tolerance(T v)
        {
            fp_tolerance<T>() = v;
        }

        static void set_all_fp_tolerance(long double v)
        {
            fp_tolerance<float>() = static_cast<float>(v);
            fp_tolerance<double>() = static_cast<double>(v);
            fp_tolerance<long double>() = v;
        }

    private:
        config() = default;

        static config &instance()
        {
            static config inst;
            return inst;
        }

        double roi_{0.0};
        double warmup_{0.0};
        uint64_t warmup_iters_{100};
        uint64_t bench_iters_{100};
    };

    /*
    Singleton Class for the current state
    */
    class current_state
    {
    public:
        current_state(const current_state &) = delete;
        current_state(current_state &&) = delete;
        current_state &operator=(const current_state &) = delete;
        current_state &operator=(current_state &&) = delete;

        static bool using_plugin() { return instance().using_plugin_; }
        static void set_using_plugin(bool v) { instance().using_plugin_ = v; }

        static std::string_view impl_name() { return instance().impl_name_; }
        static void set_impl_name(std::string_view name) { instance().impl_name_ = name; }

    private:
        current_state() = default;

        static current_state &instance()
        {
            static current_state inst;
            return inst;
        }

        bool using_plugin_{false};
        std::string_view impl_name_;
    };

}
