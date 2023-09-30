#pragma once

#include <optional>
#include <utility>
#include <string>
#include <vector>

namespace cppargs {

    // Server as a regular void
    struct Unit {};

} // namespace cppargs

namespace cppargs::dtl {
    template <class>
    struct Is_parameter_type : std::false_type {};

    template <>
    struct Is_parameter_type<std::string> : std::true_type {};

    template <>
    struct Is_parameter_type<Unit> : std::true_type {};

    template <std::integral T>
    struct Is_parameter_type<T> : std::true_type {};

    template <class T>
    struct Is_parameter_type<std::vector<T>> : Is_parameter_type<T> {};
} // namespace cppargs::dtl

namespace cppargs {

    template <class T>
    concept parameter_type = dtl::Is_parameter_type<T>::value;

    template <parameter_type>
    class Parameter {
        std::size_t tag;

        explicit constexpr Parameter(std::size_t const tag) noexcept : tag(tag) {}
    public:
        friend class Parameters;
        friend class Arguments;
    };

    class Parameters {
        struct Internal_parameter {
            std::string_view    long_name;
            std::optional<char> short_name;
            std::string_view    description;
            bool                is_flag {};
        };

        std::vector<Internal_parameter> m_vector;
    public:
        template <class T = Unit>
        [[nodiscard]] auto
        add(std::optional<char> const short_name,
            std::string_view const    long_name,
            std::string_view const    description = {}) -> Parameter<T>
        {
            auto const tag = m_vector.size();
            m_vector.push_back({
                .long_name   = long_name,
                .short_name  = short_name,
                .description = description,
                .is_flag     = std::is_same_v<T, Unit>,
            });
            return Parameter<T> { tag };
        }

        template <class T = Unit>
        [[nodiscard]] auto
        add(std::string_view const long_name, std::string_view const description = {})
            -> Parameter<T>
        {
            return add<T>(std::nullopt, long_name, description);
        }

        [[nodiscard]] auto help_string() const -> std::string;
        [[nodiscard]] auto size() const noexcept -> std::size_t;
    };

    class Arguments {
        std::vector<std::string_view> m_vector;
    public:
        explicit Arguments(std::vector<std::string_view>&&) noexcept;

        [[nodiscard]] auto argv_0() const -> std::string_view;

        template <class T>
        [[nodiscard]] auto operator[](Parameter<T> const& parameter) const -> std::optional<T>
        {
            return std::nullopt; // TODO
        }
    };

    auto parse(int argc, char const* const* argv, Parameters const&) -> Arguments;

} // namespace cppargs
