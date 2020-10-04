#pragma once

#include <algorithm>
#include <concepts>
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

        struct bound_power_fn {
            double exponent;

            constexpr
            auto operator()(const std::integral auto& e) const noexcept {
                return std::pow(e, exponent);
            }
        };

        struct power_fn {
            constexpr
            auto operator()(const std::integral auto& exponent) const noexcept {
                return bound_power_fn(exponent);
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
        struct bound_print_to_fn {
            T& stream_ref;

            template <typename... Ts>
            void operator()(Ts&& ... ts) const {
                (stream_ref << ... << ts);
            }
        };

        struct print_to_fn {
            template <typename T>
            auto operator()(T&& t) const noexcept {
                return bound_print_to_fn<T>(t);
            }
        };

        template <template <typename> typename T>
        struct to_fn { };
    }

    namespace utility {
        template <typename... Ts>
        constexpr bool always_false = false;

        template <template <typename> typename... Ts>
        constexpr bool always_false_template = false;
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

    inline constexpr detail::functors::bound_power_fn square =
            detail::functors::power_fn{}(2);

    template <template <typename> typename T>
    inline constexpr detail::functors::to_fn<T> to;
}

template <template <typename> typename T>
auto operator|(std::ranges::range auto&& rng, detail::functors::to_fn<T>) {
    using namespace std::ranges;

    using range_type = std::remove_cvref_t<decltype(rng)>;
    using value_type = range_value_t<range_type>;

    if constexpr(std::copyable<iterator_t<range_type>>) {
        auto common = rng | views::common;
        return T<value_type>(begin(common), end(common));
    } else {
        constexpr bool addable_via_add = requires(T<value_type> t) {
            t.add(std::declval<value_type>());
        };

        constexpr bool addable_via_push_back = requires(T<value_type> t) {
            t.push_back(std::declval<value_type>());
        };

        constexpr bool addable_via_insert = requires(T<value_type> t) {
            t.insert(std::declval<value_type>());
        };

        auto container = T<value_type>();

        if constexpr(addable_via_add) {
            for (auto&& e : rng) container.add(e);
        } else if constexpr (addable_via_push_back) {
            for (auto&& e : rng) container.push_back(e);
        } else if constexpr (addable_via_insert) {
            for (auto&& e : rng) container.insert(e);
        } else {
            static_assert(
                    detail::utility::always_false_template<T>,
                    "Rquired add(), push_back() or insert() methods for passed "
                    "container that accept the value type"
            );
        }

        return container;
    }

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
