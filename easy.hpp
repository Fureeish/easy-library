#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <cmath>
#include <random>
#include <ranges>
#include <string>

namespace detail {
    namespace random {
        using generator = std::mt19937_64;

        auto get_random_data() {
            using data_type = typename generator::result_type;
            std::array<data_type, generator::state_size> data;
            std::random_device rd;

            std::ranges::generate(data, std::ref(rd));

            return data;
        }

        auto get_seeded_generator() {
            using namespace std::ranges;
            const auto data = get_random_data();
            std::seed_seq seeds(begin(data), end(data));

            generator gen(seeds);

            return gen;
        }
    }

    namespace functors {
        struct even_fn {
            constexpr
            bool operator()(const std::integral auto& e) const noexcept {
                return e % 2 == 0;
            }
        };

        struct binded_power_fn {
            double exponent;

            constexpr
            auto operator()(const std::integral auto& e) const noexcept {
                return std::pow(e, exponent);
            }
        };

        struct power_fn {
            constexpr
            auto operator()(const std::integral auto& exponent) const noexcept {
                return binded_power_fn(exponent);
            }
        };

        template <typename T>
        struct construct_fn {
            template <typename... Ts>
            T operator()(Ts&& ... ts) const noexcept(noexcept(T(
                    std::forward<Ts>(ts)...))) {
                return T(std::forward<Ts>(ts)...);
            }
        };

        struct print_fn {
            template <typename... Ts>
            void operator()(Ts&& ... ts) const {
                (std::cout << ... << ts);
            }
        };

        template <typename T>
        struct binded_print_to_fn {
            T& stream_ref;

            template <typename... Ts>
            void operator()(Ts&& ... ts) const {
                (stream_ref << ... << ts);
            }
        };

        struct print_to_fn {
            template <typename T>
            auto operator()(T&& t) const noexcept {
                return binded_print_to_fn<T>(t);
            }
        };
    }
}

namespace easy {
    thread_local std::mt19937_64 random_engine =
            detail::random::get_seeded_generator();

    template <typename T>
    inline constexpr detail::functors::construct_fn<T> construct;

    inline constexpr detail::functors::print_fn print;

    inline constexpr detail::functors::print_to_fn print_to;

    inline constexpr detail::functors::even_fn even;

    inline constexpr detail::functors::power_fn pow;

    inline constexpr detail::functors::binded_power_fn square =
            detail::functors::power_fn{}(2);
}

std::ostream& operator<<(
        std::ostream& out,
        std::ranges::range auto&& range
) requires (!std::is_convertible_v<decltype(range), std::string>) {
    using namespace std::ranges;

    auto current = begin(range);

    if (current == end(range)) {
        return out << "[]";
    }

    out << '[' << *current;
    while (++current != end(range)) {
        out << ',' << *current;
    }

    return out << ']';
}
