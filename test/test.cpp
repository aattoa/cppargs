#include <catch2/catch_test_macros.hpp>
#include <cppargs.hpp>

TEST_CASE("cppargs::Parameters::help_string", "[cppargs]")
{
    SECTION("non-empty")
    {
        cppargs::Parameters parameters;

        (void)parameters.add("help", "Show this help text");
        (void)parameters.add<int>("do-thing");
        (void)parameters.add<std::string>('i', "interesting", "Do interesting things");
        (void)parameters.add("version", "Show version information");

        REQUIRE(
            parameters.help_string()
            == "\t--help                  : Show this help text\n"
               "\t--do-thing [arg]        : ...\n"
               "\t--interesting, -i [arg] : Do interesting things\n"
               "\t--version               : Show version information\n");
    }
    SECTION("empty")
    {
        REQUIRE(cppargs::Parameters().help_string().empty());
    }
}

TEST_CASE("cppargs::parse", "[cppargs]")
{
    char const* const command_line[] { "cppargstest", "--version" };

    cppargs::Parameters parameters;
    auto const          help_flag    = parameters.add("help");
    auto const          version_flag = parameters.add("version");

    cppargs::Arguments const arguments
        = cppargs::parse(std::size(command_line), command_line, parameters);

    REQUIRE(arguments.argv_0() == "cppargstest");
    REQUIRE(arguments[version_flag].has_value());
    REQUIRE_FALSE(arguments[help_flag].has_value());
}
