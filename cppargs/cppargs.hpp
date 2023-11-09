#pragma once

#include <type_traits>
#include <exception>
#include <optional>
#include <charconv>
#include <utility>
#include <memory>
#include <string>
#include <vector>
#include <span>

namespace cppargs {

    // Information needed to produce an error message
    struct Parse_error_info {
        enum class Kind {
            unrecognized_option,
            missing_argument,
            invalid_argument,
            positional_argument,
        };

        std::string command_line;
        Kind        kind {};
        std::size_t error_column {};
        std::size_t error_width {};

        static auto kind_to_string(Kind) -> std::string_view;
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

    using Command_line = std::span<char const* const>;

    // Regular void
    struct Unit {};

    template <class T>
    class Parameter {
        std::unique_ptr<std::optional<T>> m_value;
        friend class Parameters;
    public:
        [[nodiscard]] auto value() const noexcept -> T const&
        {
            return m_value->value();
        }

        [[nodiscard]] auto has_value() const noexcept -> bool
        {
            return m_value->has_value();
        }

        [[nodiscard]] explicit operator bool() const noexcept
        {
            return has_value();
        }
    };

    template <class>
    struct Argument {};

    template <class T>
    concept argument = requires(std::string_view const view) {
        // clang-format off
        { Argument<T>::parse(view) } -> std::same_as<std::optional<T>>;
        // clang-format on
    };

    namespace dtl {
        struct Parameter_info {
            using Parse = auto(std::string_view, void*) -> bool;
            Parse*              parse {};
            void*               value {};
            bool                is_flag {};
            std::string_view    long_name;
            std::optional<char> short_name;
            std::string_view    description;
        };
    } // namespace dtl

    class Parameters {
        std::vector<dtl::Parameter_info> m_vector;
    public:
        [[nodiscard]] auto help_string() const -> std::string;
        [[nodiscard]] auto info_span() const noexcept -> std::span<dtl::Parameter_info const>;

        template <argument T = Unit>
        [[nodiscard]] auto add(
            std::optional<char> const short_name,
            std::string_view const    long_name,
            std::string_view const    description = {}) -> Parameter<T>
        {
            static constexpr dtl::Parameter_info::Parse* parse
                = [](std::string_view const view, void* const where) {
                      auto& optional = *static_cast<std::optional<T>*>(where);
                      return (optional = Argument<T>::parse(view)).has_value();
                  };

            Parameter<T> parameter;
            parameter.m_value = std::make_unique<std::optional<T>>();

            m_vector.push_back({
                .parse       = parse,
                .value       = parameter.m_value.get(),
                .is_flag     = std::is_same_v<T, Unit>,
                .long_name   = long_name,
                .short_name  = short_name,
                .description = description,
            });

            return parameter;
        }

        template <argument T = Unit>
        [[nodiscard]] auto add(
            std::string_view const long_name, std::string_view const description = {})
            -> Parameter<T>
        {
            return add<T>(std::nullopt, long_name, description);
        }
    };

    auto parse(Command_line command_line, Parameters const& parameters) -> void;

    auto parse(int argc, char const* const* argv, Parameters const& parameters) -> void;

} // namespace cppargs

template <>
struct cppargs::Argument<cppargs::Unit> {
    static auto parse(std::string_view) -> std::optional<Unit>
    {
        return Unit {};
    }
};

template <>
struct cppargs::Argument<std::string_view> {
    static auto parse(std::string_view const view) -> std::optional<std::string_view>
    {
        return view;
    }
};

template <>
struct cppargs::Argument<std::string> {
    static auto parse(std::string_view const view) -> std::optional<std::string>
    {
        return std::string(view);
    }
};

template <>
struct cppargs::Argument<char> {
    static auto parse(std::string_view const view) -> std::optional<char>
    {
        if (view.size() == 1) {
            return view.front();
        }
        else {
            return std::nullopt;
        }
    }
};

template <>
struct cppargs::Argument<bool> {
    static auto parse(std::string_view const view) -> std::optional<bool>
    {
        if (view == "true" || view == "yes" || view == "on" || view == "1") {
            return true;
        }
        else if (view == "false" || view == "no" || view == "off" || view == "0") {
            return false;
        }
        else {
            return std::nullopt;
        }
    }
};

template <std::integral T>
struct cppargs::Argument<T> {
    static auto parse(std::string_view const view) -> std::optional<T>
    {
        auto const begin = view.data();
        auto const end   = begin + view.size();

        T value;
        auto const [ptr, ec] = std::from_chars(begin, end, value);

        if (ptr == end && ec == std::errc {}) {
            return value;
        }
        else {
            return std::nullopt;
        }
    }
};
