#pragma once

#include <optional>
#include <variant>
#include <utility>
#include <cstdint>
#include <string>
#include <vector>

namespace cppargs {

    template <class T>
    class Parameter {
        std::size_t tag;

        explicit Parameter(std::size_t const tag) noexcept : tag(tag) {}
    public:
        friend class Parameters;
    };

    // Serves as a regular void
    struct Unit {
        auto operator<=>(Unit const&) const = default;
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
            Detail<T>        detail;
            std::string_view name;
        };

        std::vector<std::variant<
            Internal_parameter<std::int64_t>,
            Internal_parameter<double>,
            Internal_parameter<std::string_view>,
            Internal_parameter<bool>,
            Internal_parameter<Unit>>>
            vector;
    public:
        template <class T = Unit>
        [[nodiscard]] auto add(std::string_view const name, Detail<T> const detail = {})
            -> Parameter<T>
        {
            auto const tag = vector.size();
            vector.push_back(Internal_parameter<T> { detail, name });
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
