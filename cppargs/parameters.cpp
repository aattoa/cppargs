#include <cppargs.hpp>
#include <format>

auto cppargs::Parameters::add_parameter(
    std::optional<char> const            short_name,
    std::string_view const               long_name,
    std::string_view const               description,
    dtl::Parameter_info::Validate* const validate) -> std::size_t
{
    auto const index = m_vector.size() + 1;
    m_vector.push_back({
        .validate    = validate,
        .long_name   = long_name,
        .short_name  = short_name,
        .description = description,
        .index       = index,
    });
    return index;
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
        if (parameter.validate != nullptr) {
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
