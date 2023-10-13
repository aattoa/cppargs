#include <cppargs.hpp>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <ranges>
#include <format>

namespace {
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
            "Error: Missing argument for parameter '{}'", error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::unrecognized_option:
        m_exception_string
            = std::format("Error: Unrecognized option '{}'", error_substring(m_parse_error_info));
        return;
    case Parse_error_info::Kind::positional_argument:
        m_exception_string = std::format(
            "Error: Positional arguments are not supported yet: '{}'",
            error_substring(m_parse_error_info));
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
            line.append(" [arg]");
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

    std::vector<std::optional<std::string_view>> arguments(parameters.info_span().size() + 1);
    std::size_t                                  current_column = 1;

    if (command_line.front() != nullptr) {
        std::string_view const string = command_line.front();
        arguments.front()             = string;
        current_column += (string.size() + 1);
    }

    for (auto arg_it = command_line.begin() + 1; arg_it != command_line.end(); ++arg_it) {
        std::string_view const string = *arg_it;

        auto const exception = [&](Parse_error_info::Kind const kind, std::string_view const view) {
            assert(string.find(view) != std::string_view::npos);
            return Exception { Parse_error_info {
                .command_line = make_command_line_string(command_line),
                .kind         = kind,
                .error_column = current_column + std::distance(string.begin(), view.begin()),
                .error_width  = view.size(),
            } };
        };

        if (string.starts_with("--")) {
            auto const name = string.substr(2);
            auto const it
                = std::ranges::find(parameters.info_span(), name, &dtl::Parameter_info::long_name);

            if (it == parameters.info_span().end()) {
                throw exception(Parse_error_info::Kind::unrecognized_option, name);
            }
            else if (it->is_flag) {
                arguments.at(it->index).emplace();
            }
            else if (++arg_it != command_line.end()) {
                arguments.at(it->index) = *arg_it;
            }
            else {
                throw exception(Parse_error_info::Kind::missing_argument, name);
            }
        }
        else if (string.starts_with('-')) {
            for (auto char_it = string.begin() + 1; char_it != string.end(); ++char_it) {
                auto const name = std::string_view(char_it, 1);
                auto const it   = std::ranges::find(
                    parameters.info_span(), *char_it, &dtl::Parameter_info::short_name);

                if (it == parameters.info_span().end()) {
                    throw exception(Parse_error_info::Kind::unrecognized_option, name);
                }
                else if (it->is_flag) {
                    arguments.at(it->index).emplace();
                }
                else if (char_it + 1 != string.end()) {
                    arguments.at(it->index).emplace(char_it + 1, string.end());
                    break;
                }
                else if (++arg_it != command_line.end()) {
                    arguments.at(it->index) = *arg_it;
                }
                else {
                    throw exception(Parse_error_info::Kind::missing_argument, name);
                }
            }
        }
        else {
            throw exception(Parse_error_info::Kind::positional_argument, string);
        }

        current_column += (string.size() + 1);
    }

    return Arguments { std::move(arguments) };
}
