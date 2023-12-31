#include <cppargs.hpp>
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

    [[nodiscard]] auto command_line_column(
        cppargs::Command_line::iterator const begin, cppargs::Command_line::iterator const end)
        -> std::size_t
    {
        std::size_t column = 1;
        for (auto it = begin; it != end; ++it) {
            column += (*it == nullptr ? 0 : 1 + std::string_view(*it).size());
        }
        return column;
    }
} // namespace

auto cppargs::parse(Command_line const command_line, Parameters const& parameters) -> void
{
    if (!is_valid_command_line(command_line)) {
        throw std::invalid_argument { "cppargs::parse: Invalid command line" };
    }

    for (auto arg_it = command_line.begin() + 1; arg_it != command_line.end(); ++arg_it) {
        std::string_view const string = *arg_it;

        auto const exception = [&](Parse_error_info::Kind const kind,
                                   std::string_view const       view,
                                   std::size_t const            column_plus) {
            return Exception { Parse_error_info {
                .command_line = make_command_line_string(command_line),
                .kind         = kind,
                .error_column = command_line_column(command_line.begin(), arg_it) + column_plus,
                .error_width  = view.size(),
            } };
        };

        if (string != "--" && string.starts_with("--")) {
            auto const name = string.substr(2);
            auto const it
                = std::ranges::find(parameters.info_span(), name, &dtl::Parameter_info::long_name);

            if (it == parameters.info_span().end()) {
                throw exception(Parse_error_info::Kind::unrecognized_option, name, 2);
            }
            else if (it->is_flag) {
                (void)it->parse({}, it->value);
            }
            else if (arg_it + 1 == command_line.end()) {
                throw exception(Parse_error_info::Kind::missing_argument, name, 2);
            }
            else if (!it->parse(*++arg_it, it->value)) {
                throw exception(Parse_error_info::Kind::invalid_argument, *arg_it, 0);
            }
        }
        else if (string != "--" && string != "-" && string.starts_with('-')) {
            std::size_t offset = 1;
            for (auto char_it = string.begin() + 1; char_it != string.end(); ++char_it, ++offset) {
                auto const name = std::string_view(char_it, 1);
                auto const it   = std::ranges::find(
                    parameters.info_span(), *char_it, &dtl::Parameter_info::short_name);

                if (it == parameters.info_span().end()) {
                    throw exception(Parse_error_info::Kind::unrecognized_option, name, offset);
                }
                else if (it->is_flag) {
                    (void)it->parse({}, it->value);
                }
                else if (char_it + 1 != string.end()) {
                    std::string_view const argument(char_it + 1, string.end());
                    if (it->parse(argument, it->value)) {
                        break;
                    }
                    throw exception(Parse_error_info::Kind::invalid_argument, argument, offset + 1);
                }
                else if (arg_it + 1 == command_line.end()) {
                    throw exception(Parse_error_info::Kind::missing_argument, name, offset);
                }
                else if (!it->parse(*++arg_it, it->value)) {
                    throw exception(Parse_error_info::Kind::invalid_argument, *arg_it, 0);
                }
            }
        }
        else {
            throw exception(Parse_error_info::Kind::positional_argument, string, 0);
        }
    }
}

auto cppargs::parse(int const argc, char const* const* const argv, Parameters const& parameters)
    -> void
{
    assert(argc != 0);
    assert(argv != nullptr);
    return parse(Command_line(argv, argv + argc), parameters);
}
