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
 * @file concepts.hpp
 * @brief This file contains commonly used concepts internally within the ComPPare library.
 */

#pragma once
#include <concepts>
#include <type_traits>
#include <ranges>
#include <string>
#include <ostream>

/**
 * @namespace comppare::internal::concepts
 * @brief Contains commonly used concepts internally within the ComPPare library.
 */
namespace comppare::internal::concepts
{
    /**
     * @brief Concept for a type that can be streamed to an output stream.
     * 
     * @tparam T 
     */
    template <typename T>
    concept Streamable =
        requires(std::ostream &os, T v) { { os << v } -> std::same_as<std::ostream&>; };

    /**
     * @brief Concept for a floating-point type.
     * 
     * @tparam T 
     */
    template <typename T>
    concept FloatingPoint = std::floating_point<std::remove_cvref_t<T>>;

    /**
     * @brief Concept for an integral type.
     * 
     * @tparam T 
     */
    template <typename T>
    concept Integral = std::integral<std::remove_cvref_t<T>>;


    /**
     * @brief Concept for an arithmetic type (either floating-point or integral).
     * 
     * @tparam T 
     */
    template <typename T>
    concept Arithmetic = FloatingPoint<T> || Integral<T>;


    /**
     * @brief Concept for a string type.
     * 
     * @tparam T 
     */
    template <typename T>
    concept String = std::same_as<std::remove_cvref_t<T>, std::string>;

    /**
     * @brief Concept for a void type.
     * 
     * @tparam T 
     */
    template <typename T>
    concept Void = std::is_void_v<std::remove_cvref_t<T>>;

    /**
     * @brief Concept for a range of arithmetic types, excluding strings.
     * 
     * @tparam R 
     */
    template <typename R>
    concept RangeOfArithmetic =
        std::ranges::forward_range<std::remove_cvref_t<R>> && Arithmetic<std::remove_cvref_t<std::ranges::range_value_t<std::remove_cvref_t<R>>>> && (!String<std::remove_cvref_t<R>>);

}