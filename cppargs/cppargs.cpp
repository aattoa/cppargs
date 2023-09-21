#include <cppargs.hpp>
#include <type_traits>
#include <algorithm>
#include <format>

namespace {
    template <class T>
    constexpr auto type_description() noexcept
    {
        if constexpr (std::is_same_v<T, cppargs::Int>) {
            return "int";
        }
        else if constexpr (std::is_same_v<T, cppargs::Float>) {
            return "float";
        }
        else if constexpr (std::is_same_v<T, cppargs::Str>) {
            return "str";
        }
        else if constexpr (std::is_same_v<T, cppargs::Bool>) {
            return "bool";
        }
        else if constexpr (std::is_same_v<T, cppargs::Unit>) {
            return "unit";
        }
        else {
            static_assert(sizeof(T) == 0);
        }
    }
} // namespace

auto cppargs::parse(int const argc, char const* const* const argv, Parameters const& parameters)
    -> Arguments
{
    (void)argc;
    (void)argv;
    (void)parameters;
    std::abort();
}

auto cppargs::Parameters::help_string() const -> std::string
{
    std::vector<std::pair<std::string, std::string_view>> lines;
    std::size_t                                           max_length {};

    for (auto const& variant : vector) {
        auto const visitor = [&]<class T>(Internal_parameter<T> const& parameter) {
            std::string line = std::format("--{}", parameter.long_name);
            if (parameter.short_name.has_value()) {
                std::format_to(std::back_inserter(line), ", -{}", parameter.short_name.value());
            }
            if constexpr (!std::is_same_v<T, Unit>) {
                std::format_to(std::back_inserter(line), " [{}]", type_description<T>());
            }
            max_length = std::max(max_length, line.size());
            lines.emplace_back(std::move(line), parameter.detail.description);
        };
        std::visit(visitor, variant);
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
