#include <cppargs.hpp>
#include <format>

namespace {
    [[nodiscard]] auto error_substring(cppargs::Parse_error_info const& info) -> std::string_view
    {
        return std::string_view(info.command_line).substr(info.error_column - 1, info.error_width);
    }
} // namespace

cppargs::Exception::Exception(Parse_error_info&& parse_error_info)
    : m_parse_error_info(std::move(parse_error_info))
{
    switch (m_parse_error_info.kind) {
    case Parse_error_info::Kind::missing_argument:
        m_exception_string = std::format(
            "Missing argument for parameter '{}'", error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::invalid_argument:
        m_exception_string
            = std::format("Invalid argument '{}'", error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::unrecognized_option:
        m_exception_string
            = std::format("Unrecognized option '{}'", error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::positional_argument:
        m_exception_string = std::format(
            "Positional arguments are not supported yet: '{}'",
            error_substring(m_parse_error_info));
        return;
    default:
        throw std::invalid_argument {
            "cppargs::Exception::Exception: Invalid Parse_error_info::Kind enumerator value"
        };
    }
}

auto cppargs::Exception::info() const noexcept -> Parse_error_info const&
{
    return m_parse_error_info;
}

auto cppargs::Exception::what() const noexcept -> char const*
{
    return m_exception_string.data();
}
