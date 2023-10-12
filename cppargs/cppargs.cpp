#include <cppargs.hpp>

// Only for todo()
#include <source_location>
#include <iostream>

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <ranges>
#include <format>

namespace {
    [[noreturn]] auto todo(std::source_location const caller = std::source_location::current())
        -> void
    {
        std::cerr << "Unimplemented branch reached on line " << caller.line() << ", in file "
                  << caller.file_name() << std::endl;
        std::exit(-1);
    }

    [[nodiscard]] auto is_valid_command_line(cppargs::Command_line const command_line) noexcept
        -> bool
    {
        // `argv[0]` is allowed to be null.
        return !command_line.empty()
            && std::all_of(command_line.begin() + 1, command_line.end(), [](auto const ptr) {
                   return ptr != nullptr;
               });
    }

    [[nodiscard]] auto make_command_line_string(cppargs::Command_line const command_line)
        -> std::string
    {
        assert(is_valid_command_line(command_line));
        std::string line = command_line.front();
        std::for_each(
            command_line.begin() + 1, command_line.end(), [&line](char const* const string) {
                line.append(1, ' ').append(string);
            });
        return line;
    }

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
            "Error: cppargs: Missing argument for parameter '{}'",
            error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::unrecognized_option:
        m_exception_string = std::format(
            "Error: cppargs: Unrecognized option '{}'", error_substring(m_parse_error_info));
        return;
    default:
        throw std::invalid_argument {
            "cppargs::Exception::Exception: "
            "Invalid Parse_error_info::Kind enumerator value ",
        };
    }
}

auto cppargs::Exception::info() const noexcept -> Parse_error_info const&
{
    return m_parse_error_info;
}

auto cppargs::Exception::what() const noexcept -> char const*
{
    assert(!m_exception_string.empty());
    return m_exception_string.data();
}

auto cppargs::Parameters::help_string() const -> std::string
{
    std::vector<std::pair<std::string, std::string_view>> lines;
    std::size_t                                           max_length {};

    for (auto const& parameter : m_vector) {
        std::string line = std::format("--{}", parameter.long_name);
        if (parameter.short_name.has_value()) {
            std::format_to(std::back_inserter(line), ", -{}", parameter.short_name.value());
        }
        if (!parameter.is_flag) {
            line += " [arg]";
        }
        max_length = std::max(max_length, line.size());
        lines.emplace_back(std::move(line), parameter.description);
    }

    std::string string;
    for (auto const& [names, description] : lines) {
        std::format_to(
            std::back_inserter(string),
            "\t{:{}} : {}\n",
            names,
            max_length,
            description.empty() ? "..." : description);
    }
    return string;
}

auto cppargs::Parameters::info_span() const noexcept -> std::span<dtl::Parameter_info const>
{
    return m_vector;
}

auto cppargs::Arguments::argv_0() const -> std::optional<std::string_view>
{
    return m_vector.front();
}

auto cppargs::parse(Command_line const command_line, Parameters const& parameters) -> Arguments
{
    if (!is_valid_command_line(command_line)) {
        throw std::invalid_argument { "cppargs::parse: Invalid command line" };
    }

    std::vector<std::optional<std::string_view>> argument_vector(parameters.info_span().size() + 1);
    std::size_t                                  current_column = 1;

    if (command_line.front() != nullptr) {
        std::string_view const string = command_line.front();
        argument_vector.front()       = string;
        current_column += (string.size() + 1);
    }

    for (auto arg_it = command_line.begin() + 1; arg_it != command_line.end(); ++arg_it) {
        std::string_view string = *arg_it;

        if (string.starts_with("--")) {
            string.remove_prefix(2);

            auto const exception = [&](Parse_error_info::Kind const kind) {
                return Exception { Parse_error_info {
                    .command_line = make_command_line_string(command_line),
                    .kind         = kind,
                    .error_column = current_column + 2, // +2 for the "--"
                    .error_width  = string.size(),
                } };
            };

            auto const it = std::ranges::find(
                parameters.info_span(), string, &dtl::Parameter_info::long_name);

            if (it == parameters.info_span().end()) {
                throw exception(Parse_error_info::Kind::unrecognized_option);
            }

            if (it->is_flag) {
                argument_vector.at(it->index).emplace();
            }
            else if (++arg_it != command_line.end()) {
                argument_vector.at(it->index) = *arg_it;
            }
            else {
                throw exception(Parse_error_info::Kind::missing_argument);
            }
        }
        else if (string.starts_with('-')) {
            todo();
        }
        else {
            todo();
        }

        current_column += (string.size() + 1);
    }

    return Arguments { std::move(argument_vector) };
}
