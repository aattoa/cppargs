#include <cppargs.hpp>

#include <type_traits>
#include <algorithm>
#include <cassert>
#include <format>

cppargs::Arguments::Arguments(std::vector<std::string_view>&& vector) noexcept
    : m_vector(std::move(vector))
{
    assert(!m_vector.empty());
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

auto cppargs::Parameters::size() const noexcept -> std::size_t
{
    return m_vector.size();
}

auto cppargs::Arguments::argv_0() const -> std::string_view
{
    assert(!m_vector.empty());
    return m_vector.front();
}

auto cppargs::parse(int argc, char const* const* argv, Parameters const& parameters) -> Arguments
{
    assert(argc != 0);
    assert(argv != nullptr);

    std::vector<std::string_view> vector(parameters.size() + 1);
    vector.front() = *argv;

    // TODO

    return Arguments { std::move(vector) };
}
