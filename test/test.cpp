#include <catch2/catch_test_macros.hpp>
#include <cppargs.hpp>

TEST_CASE("cppargs")
{
    char const* const command_line[] { "cppargstest", "--version" };

    cppargs::Parameters parameters;

    auto const help_flag    = parameters.add("help");
    auto const version_flag = parameters.add("version");

    cppargs::Arguments const arguments
        = cppargs::parse(std::size(command_line), command_line, parameters);

    REQUIRE_FALSE(arguments[help_flag].has_value());
    REQUIRE(arguments[version_flag].has_value());
}
