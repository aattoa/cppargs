#pragma once

#include <optional>
#include <variant>
#include <utility>
#include <cstdint>
#include <string>
#include <vector>

namespace cppargs {

    using Int   = std::int64_t;
    using Float = double;
    using Str   = std::string_view;
    using Bool  = bool;

    // Serves as a regular void
    struct Unit {
        auto operator<=>(Unit const&) const = default;
    };

    template <class T>
    class Parameter {
        std::size_t tag;

        explicit Parameter(std::size_t const tag) noexcept : tag(tag) {}
    public:
        friend class Parameters;
    };

    using Flag = Parameter<Unit>;

    class Parameters {
    public:
        template <class T>
        struct Detail {
            std::optional<T> minimum_value;
            std::optional<T> maximum_value;
            std::optional<T> default_value;
            std::string_view description;
        };
    private:
        template <class T>
        struct Internal_parameter {
            Detail<T>           detail;
            std::string_view    long_name;
            std::optional<char> short_name;
        };

        std::vector<std::variant<
            Internal_parameter<Int>,
            Internal_parameter<Float>,
            Internal_parameter<Str>,
            Internal_parameter<Bool>,
            Internal_parameter<Unit>>>
            vector;
    public:
        template <class T = Unit>
        [[nodiscard]] auto add(std::string_view const name, Detail<T> const detail = {})
            -> Parameter<T>
        {
            auto const tag = vector.size();
            vector.push_back(Internal_parameter<T> { .detail = detail, .long_name = name });
            return Parameter<T> { tag };
        }

        [[nodiscard]] auto help_string() const -> std::string;
    };

    class Arguments {
    public:
        template <class T>
        [[nodiscard]] auto operator[](Parameter<T>) const -> std::optional<T>
        {
            std::abort();
        }
    };

    auto parse(int argc, char const* const* argv, Parameters const&) -> Arguments;

} // namespace cppargs
