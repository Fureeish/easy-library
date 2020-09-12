#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <ranges>

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
}

namespace easy {
    thread_local std::mt19937_64 random_engine =
            detail::random::get_seeded_generator();
}

std::ostream& operator<<(
        std::ostream& out,
        std::ranges::range auto&& range
) requires (!std::is_convertible_v<decltype(range), std::string>) {
    using namespace std::ranges;

    if (empty(range)) {
        return out << "[]";
    }

    auto current = begin(range);
    out << '[' << *current;

    while (++current != end(range)) {
        out << ',' << *current;
    }

    return out << ']';
}

