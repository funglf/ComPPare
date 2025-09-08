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
 * @file ansi.hpp
 * @author Leong Fan FUNG (funglf) <stanleyfunglf@gmail.com>
 * @brief This file contains utilities for applying ANSI styles and colors to console output.
 * @date 2025
 * @copyright MIT License
 * @see LICENSE For full license text.
 *
 * ## Usage
 * This file supports two usage patterns for formatting console output with ANSI escape codes:
 *
 * ### 1. Persistent toggles
 * - Applied with stream insertion (`<<`).
 * - Remain active until explicitly turned off with the corresponding `_OFF` code.
 * - Examples: `BOLD` / `BOLD_OFF`, `RED_` / `RED_OFF`.
 *
 * ```cpp
 * std::cout << comppare::internal::ansi::BOLD
 *           << comppare::internal::ansi::RED
 *           << "Bold Red"
 *           << comppare::internal::ansi::RED_OFF
 *           << comppare::internal::ansi::BOLD_OFF
 *           << " Back to normal";
 * ```
 *
 * ### 2. Scoped wrappers
 * - Applied with function-call syntax, e.g. `RED("Red")`.
 * - Wrap the string with an "on" and "off" code automatically.
 * - Useful for one-off highlighted messages.
 *
 * ```cpp
 * std::cout << "Normal Colour"
 *           << comppare::internal::ansi::RED("Red")
 *           << "Normal Colour" << "\n";
 * ```
 *
 * ### 3. Reset code
 * - `RESET` can be used to reset all styles and colors to default.
 * std::cout << comppare::internal::ansi::BOLD
 *           << comppare::internal::ansi::RED
 *           << "Bold Red"
 *           << comppare::internal::ansi::RESET
 *           << " Back to normal";
 */
#pragma once
#include <ostream>
#include <sstream>
#include <string>
#include <concepts>
#include <utility>

#include <comppare/internal/concepts.hpp>

/**
 * @namespace comppare::internal::ansi
 * @brief Utilities for applying ANSI styles and colors to console output.
 *
 * This namespace provides helpers for applying ANSI escape codes to console output.
 *
 */
namespace comppare::internal::ansi
{
    /**
     * @brief Applying ANSI styles/colors to a value in a scope.
     *
     * The `ScopedAnsiWrapper` class applies an ANSI "on" code before the value
     * and an "off" code immediately after. This makes styles and colors *scoped*
     * to the wrapped value, unlike persistent toggles.
     *
     * Example:
     * ```cpp
     * std::cout << comppare::internal::ansi::BOLD("Bold Text")
     *           << " normal text";
     * ```
     * The text `"Bold Text"` is printed in bold, and the style is automatically
     * reset afterwards.
     *
     * @tparam T The type of the value to be wrapped. Must satisfy
     *           `Streamable` (i.e. it can be inserted into a `std::ostream`).
     *
     * ### Implementation Details
     * #### Private Data Members:
     *   1. `on_`  – ANSI code to enable the style/color
     *   2. `off_` – ANSI code to disable it
     *   3. `val_` – the wrapped value
     * #### `operator<<`:
     *   1. Saves the current formatting state of the stream with copyfmt
     *   2. Streams the value `val_` into a temporary `std::ostringstream`
     *   3. Writes `on_ + value + off_` to the original stream.
     *   4. Restores the saved formatting state so it does not affect formatting.
     *
     * **Friend `operator<<`**
     * The insertion operator is declared as a `friend` so it can access
     * the wrapper’s private members on_, off_, and val_.:
     * ```cpp
     * friend std::ostream &operator<<(std::ostream &os,
     *                                 ScopedAnsiWrapper const &w);
     * ```
     * This allows syntax like:
     * ```cpp
     * std::cout << RED("Red") << " Normal";
     * ```
     */
    template <comppare::internal::concepts::Streamable T>
    class ScopedAnsiWrapper
    {
        /** @brief ANSI "on" code */
        const char *on_;
        /** @brief ANSI "off" code */
        const char *off_;
        /** @brief The value to be wrapped */
        T val_;

    public:
        ScopedAnsiWrapper(const char *on, const char *off, T v)
            : on_(on), off_(off), val_(std::move(v)) {}

        /**
         * @brief Overloaded operator<< to stream the value with ANSI codes.
         *
         * @param os The output stream
         * @param w The ScopedAnsiWrapper instance
         * @return std::ostream& The modified output stream
         */
        friend std::ostream &operator<<(std::ostream &os, ScopedAnsiWrapper const &w)
        {
            // Save the current state of the stream
            std::ios saved(nullptr);
            saved.copyfmt(os);

            // convert the value into a string
            std::ostringstream tmp;
            tmp.copyfmt(os);
            tmp << w.val_;
            std::string body = std::move(tmp).str();

            // apply the ANSI codes
            os.width(0);
            os << w.on_ << body << w.off_;

            // Restore the original state of the stream -- formatting, precision, etc.
            os.copyfmt(saved);
            return os;
        }
    };

    /**
     * @brief Macro to define ANSI escape codes for text styling and colors.
     *
     * @details
     * This macro generates a *pair of classes* and its *instances* to
     * represent an ANSI style or color. The resulting API supports both:
     * - **Persistent toggles** (turn style/color on or off in the stream)
     * - **Scoped wrappers** (apply ON/OFF just around a single value)
     *
     * ---
     * ### Implementation Details
     *
     * Example with the style `BOLD`:
     * 1. **On Off codes**
     *    - `NAME##_ON_CODE`  → string literal for enabling (`"\033[1m"`)
     *    - `NAME##_OFF_CODE` → string literal for disabling (`"\033[22m"`)
     * ECMA-48 “Select Graphic Rendition” (SGR) sequences are defined here:
     * https://man7.org/linux/man-pages/man4/console_codes.4.html
     *
     * 2. **ON class** (`NAME##_ON_t`)
     *    - `operator<<` — inserts the ON code directly into an `ostream`
     *    - `operator()` — wraps a value in a `ScopedAnsiWrapper`, which streams
     *      `ON + value + OFF` automatically
     *
     *    ```cpp
     *    class BOLD_ON_t {
     *    public:
     *        // Toggle on: std::cout << BOLD << "bold";
     *        friend std::ostream& operator<<(std::ostream&, BOLD_ON_t);
     *        // Scoped: std::cout << BOLD("bold");
     *        template<Streamable T>
     *        auto operator()(T&& v) const;
     *    };
     *    ```
     *
     *    **Instance:**
     *    ```cpp
     *    inline constexpr BOLD_ON_t BOLD{};
     *    ```
     *    Users actually call this *instance*, not the class itself:
     *    ```cpp
     *    std::cout << BOLD;         // BOLD_ON_t::operator<<
     *    std::cout << BOLD("msg");  // BOLD_ON_t::operator()
     *    ```
     *
     * 3. **OFF class** (`NAME##_OFF_t`)
     *    - `operator<<` — inserts the OFF code directly into an `ostream`
     *
     *    ```cpp
     *    class BOLD_OFF_t {
     *    public:
     *        // Toggle off: std::cout << BOLD_OFF << "normal";
     *        friend std::ostream& operator<<(std::ostream&, BOLD_OFF_t);
     *    };
     *    ```
     *
     *    **Instance:**
     *    ```cpp
     *    inline constexpr BOLD_OFF_t BOLD_OFF{};
     *    ```
     *    Usage:
     *    ```cpp
     *    std::cout << BOLD_OFF; // BOLD_OFF_t::operator<<
     *    ```
     *
     * @param NAME The identifier to generate (`BOLD`, `RED`, etc.)
     * @param ON   Numeric ANSI code to enable
     * @param OFF  Numeric ANSI code to disable/reset
     */

#define ANSI_DEFINE(NAME, ON, OFF)                                                                          \
    /* Actual ANSI codes for the style/color (eg: BLACK_ON_CODE = "\033[30m") */                            \
    inline constexpr const char *NAME##_ON_CODE = "\033[" ON "m";                                           \
    inline constexpr const char *NAME##_OFF_CODE = "\033[" OFF "m";                                         \
                                                                                                            \
    class NAME##_ON_t                                                                                       \
    {                                                                                                       \
    public:                                                                                                 \
        /* Usage: std::cout << comppare::internal::ansi::BOLD << "Hello world"; */                          \
        friend std::ostream &operator<<(std::ostream &os, NAME##_ON_t)                                      \
        {                                                                                                   \
            /* Save the current state of the stream and apply the ANSI code */                              \
            std::ios saved(nullptr);                                                                        \
            saved.copyfmt(os);                                                                              \
            os.width(0);                                                                                    \
            os << NAME##_ON_CODE;                                                                           \
            os.copyfmt(saved);                                                                              \
            return os;                                                                                      \
        }                                                                                                   \
                                                                                                            \
        /* Usage: std::cout << comppare::internal::ansi::BOLD("Hello world") */                             \
        template <comppare::internal::concepts::Streamable T>                                               \
        auto operator()(T &&v) const                                                                        \
        {                                                                                                   \
            return ScopedAnsiWrapper<std::decay_t<T>>(NAME##_ON_CODE, NAME##_OFF_CODE, std::forward<T>(v)); \
        }                                                                                                   \
    };                                                                                                      \
    /* Instance of the ON class */                                                                          \
    inline constexpr NAME##_ON_t NAME{};                                                                    \
    class NAME##_OFF_t                                                                                      \
    {                                                                                                       \
    public:                                                                                                 \
        /* Usage: std::cout << ... << comppare::internal::ansi::BOLD_OFF; */                                \
        friend std::ostream &operator<<(std::ostream &os, NAME##_OFF_t)                                     \
        {                                                                                                   \
            /* Save the current state of the stream and apply the ANSI code */                              \
            std::ios saved(nullptr);                                                                        \
            saved.copyfmt(os);                                                                              \
            os.width(0);                                                                                    \
            os << NAME##_OFF_CODE;                                                                          \
            os.copyfmt(saved);                                                                              \
            return os;                                                                                      \
        }                                                                                                   \
    };                                                                                                      \
    /* Instance of the OFF class */                                                                         \
    inline constexpr NAME##_OFF_t NAME##_OFF{};

    /// @cond Actual Definitions of ANSI styles and colors
    // Each style/color is defined with its ON and OFF codes
    // https://man7.org/linux/man-pages/man4/console_codes.4.html

    /* STYLES 0-9 */
    ANSI_DEFINE(RESET, "0", "0");
    ANSI_DEFINE(BOLD, "1", "22");
    ANSI_DEFINE(DIM, "2", "22");
    ANSI_DEFINE(ITALIC, "3", "23");
    ANSI_DEFINE(UNDERLINE, "4", "24");
    ANSI_DEFINE(BLINK, "5", "25");
    ANSI_DEFINE(REVERSE, "7", "27");
    ANSI_DEFINE(HIDDEN, "8", "28");
    ANSI_DEFINE(STRIKE, "9", "29");

    /* FOREGROUND (TEXT) COLOURS 30-37/90-97 */
    ANSI_DEFINE(BLACK, "30", "39");
    ANSI_DEFINE(RED, "31", "39");
    ANSI_DEFINE(GREEN, "32", "39");
    ANSI_DEFINE(YELLOW, "33", "39");
    ANSI_DEFINE(BLUE, "34", "39");
    ANSI_DEFINE(MAGENTA, "35", "39");
    ANSI_DEFINE(CYAN, "36", "39");
    ANSI_DEFINE(WHITE, "37", "39");

    ANSI_DEFINE(BRIGHT_BLACK, "90", "39");
    ANSI_DEFINE(BRIGHT_RED, "91", "39");
    ANSI_DEFINE(BRIGHT_GREEN, "92", "39");
    ANSI_DEFINE(BRIGHT_YELLOW, "93", "39");
    ANSI_DEFINE(BRIGHT_BLUE, "94", "39");
    ANSI_DEFINE(BRIGHT_MAGENTA, "95", "39");
    ANSI_DEFINE(BRIGHT_CYAN, "96", "39");
    ANSI_DEFINE(BRIGHT_WHITE, "97", "39");

    /* BACKGROUND COLOURS (40-47 / 100-107) */
    ANSI_DEFINE(BG_BLACK, "40", "49");
    ANSI_DEFINE(BG_RED, "41", "49");
    ANSI_DEFINE(BG_GREEN, "42", "49");
    ANSI_DEFINE(BG_YELLOW, "43", "49");
    ANSI_DEFINE(BG_BLUE, "44", "49");
    ANSI_DEFINE(BG_MAGENTA, "45", "49");
    ANSI_DEFINE(BG_CYAN, "46", "49");
    ANSI_DEFINE(BG_WHITE, "47", "49");

    ANSI_DEFINE(BG_BRIGHT_BLACK, "100", "49");
    ANSI_DEFINE(BG_BRIGHT_RED, "101", "49");
    ANSI_DEFINE(BG_BRIGHT_GREEN, "102", "49");
    ANSI_DEFINE(BG_BRIGHT_YELLOW, "103", "49");
    ANSI_DEFINE(BG_BRIGHT_BLUE, "104", "49");
    ANSI_DEFINE(BG_BRIGHT_MAGENTA, "105", "49");
    ANSI_DEFINE(BG_BRIGHT_CYAN, "106", "49");
    ANSI_DEFINE(BG_BRIGHT_WHITE, "107", "49");
    /// @endcond

    // undefine the macro as no longer needed
#undef ANSI_DEFINE
}