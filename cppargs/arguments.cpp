#include <cppargs.hpp>

cppargs::Arguments::Arguments(decltype(m_vector)&& vector) noexcept : m_vector(std::move(vector)) {}

auto cppargs::Arguments::argv_0() const -> std::optional<std::string_view>
{
    return m_vector.front();
}
