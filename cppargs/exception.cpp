#include <cppargs.hpp>
#include <format>

namespace {
    [[nodiscard]] auto error_substring(cppargs::Parse_error_info const& info) -> std::string_view
    {
        return std::string_view(info.command_line).substr(info.error_column - 1, info.error_width);
    }
} // namespace

auto cppargs::Parse_error_info::kind_to_string(Kind const kind) -> std::string_view
{
    switch (kind) {
    case Parse_error_info::Kind::missing_argument:
        return "Missing argument for parameter";
    case Parse_error_info::Kind::invalid_argument:
        return "Invalid argument";
    case Parse_error_info::Kind::unrecognized_option:
        return "Unrecognized option";
    case Parse_error_info::Kind::positional_argument:
        return "Positional arguments are not supported yet";
    default:
        throw std::invalid_argument {
            "cppargs::Parse_error_info::kind_to_string: Invalid "
            "Parse_error_info::Kind enumerator value",
        };
    }
}

cppargs::Exception::Exception(Parse_error_info&& parse_error_info)
    : m_exception_string(std::format(
        "{}: '{}'",
        Parse_error_info::kind_to_string(parse_error_info.kind),
        error_substring(parse_error_info)))
    , m_parse_error_info(std::move(parse_error_info))
{}

auto cppargs::Exception::info() const noexcept -> Parse_error_info const&
{
    return m_parse_error_info;
}

auto cppargs::Exception::what() const noexcept -> char const*
{
    return m_exception_string.data();
}
