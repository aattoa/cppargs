#include <cppargs.hpp>

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
    std::abort();
}
