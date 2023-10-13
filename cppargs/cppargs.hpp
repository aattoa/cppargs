#pragma once

#include <exception>
#include <optional>
#include <utility>
#include <string>
#include <vector>
#include <span>

namespace cppargs {

    // Information needed to produce an error message
    struct Parse_error_info {
        enum class Kind {
            unrecognized_option,
            missing_argument,
            positional_argument,
        };
        std::string command_line;
        Kind        kind {};
        std::size_t error_column {};
        std::size_t error_width {};
    };

    // Thrown on parse failure
    class Exception : public std::exception {
        std::string      m_exception_string;
        Parse_error_info m_parse_error_info;
    public:
        explicit Exception(Parse_error_info&&);

        // Makes custom error message formatting possible
        [[nodiscard]] auto info() const noexcept -> Parse_error_info const&;
        [[nodiscard]] auto what() const noexcept -> char const* override;
    };

    // Regular void
    struct Unit {};

    namespace dtl {
        template <class>
        struct Is_parameter_type : std::false_type {};

        template <>
        struct Is_parameter_type<std::string> : std::true_type {};

        template <>
        struct Is_parameter_type<std::string_view> : std::true_type {};

        template <>
        struct Is_parameter_type<Unit> : std::true_type {};

        template <std::integral T>
        struct Is_parameter_type<T> : std::true_type {};

        template <class T>
        struct Is_parameter_type<std::vector<T>> : Is_parameter_type<T> {};

        template <class T>
        struct Is_parameter_type<std::vector<std::vector<T>>> : std::false_type {};

        struct Parameter_info {
            std::string_view    long_name;
            std::optional<char> short_name;
            std::string_view    description;
            bool                is_flag {};
            std::size_t         index {};
        };
    } // namespace dtl

    template <class T>
    concept parameter_type = dtl::Is_parameter_type<T>::value;

    template <parameter_type>
    class Parameter {
        std::size_t index;

        explicit constexpr Parameter(std::size_t const index) noexcept : index(index) {}
    public:
        friend class Parameters;
        friend class Arguments;
    };

    class Parameters {
        std::vector<dtl::Parameter_info> m_vector;
    public:
        template <class T = Unit>
        [[nodiscard]] auto
        add(std::optional<char> const short_name,
            std::string_view const    long_name,
            std::string_view const    description = {}) -> Parameter<T>
        {
            auto const index = m_vector.size() + 1;
            m_vector.push_back({
                .long_name   = long_name,
                .short_name  = short_name,
                .description = description,
                .is_flag     = std::is_same_v<T, Unit>,
                .index       = index,
            });
            return Parameter<T> { index };
        }

        template <class T = Unit>
        [[nodiscard]] auto
        add(std::string_view const long_name, std::string_view const description = {})
            -> Parameter<T>
        {
            return add<T>(std::nullopt, long_name, description);
        }

        [[nodiscard]] auto help_string() const -> std::string;
        [[nodiscard]] auto info_span() const noexcept -> std::span<dtl::Parameter_info const>;
    };

    using Command_line = std::span<char const* const>;

    class Arguments {
        std::vector<std::optional<std::string_view>> m_vector;

        explicit Arguments(decltype(m_vector)&& vector) noexcept : m_vector(std::move(vector)) {}

        // Only `parse` can construct `Arguments`
        friend auto parse(Command_line, Parameters const&) -> Arguments;
    public:
        // On most systems this will return the program name as it was invoked
        [[nodiscard]] auto argv_0() const -> std::optional<std::string_view>;

        // TODO: Parse argument and return optional<T>
        template <class T>
        [[nodiscard]] auto operator[](Parameter<T> const& parameter) const
            -> std::optional<std::string_view>
        {
            return m_vector.at(parameter.index);
        }
    };

    [[nodiscard]] auto parse(Command_line, Parameters const&) -> Arguments;

} // namespace cppargs
