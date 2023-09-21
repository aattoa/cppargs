#include <catch2/catch_test_macros.hpp>
#include <cppargs.hpp>

TEST_CASE("cppargs::Parameters::help_string", "[cppargs]")
{
    SECTION("non-empty")
    {
        cppargs::Parameters parameters;

        (void)parameters.add("help", { .description = "Show this help text" });
        (void)parameters.add<cppargs::Int>("do-thing");
        (void)parameters.add("version", { .description = "Show version information" });
        (void)parameters.add<cppargs::Str>(
            "interesting", { .description = "Do interesting things" });

        REQUIRE(
            parameters.help_string()
            == "\t--help              : Show this help text\n"
               "\t--do-thing [int]    : ...\n"
               "\t--version           : Show version information\n"
               "\t--interesting [str] : Do interesting things\n");
    }
    SECTION("empty")
    {
        REQUIRE(cppargs::Parameters().help_string().empty());
    }
}

TEST_CASE("cppargs::parse", "[cppargs]")
{
    char const* const   command_line[] { "cppargstest", "--version" };
    cppargs::Parameters parameters;

    auto const help_flag    = parameters.add("help");
    auto const version_flag = parameters.add("version");

    cppargs::Arguments const arguments
        = cppargs::parse(std::size(command_line), command_line, parameters);

    REQUIRE_FALSE(arguments[help_flag].has_value());
    REQUIRE(arguments[version_flag].has_value());
}
