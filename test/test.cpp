#include <catch2/catch_test_macros.hpp>
#include <cppargs.hpp>

#define REQUIRE_UNREACHABLE REQUIRE(false)
#define TEST(name)          TEST_CASE("cppargs: " name, "[cppargs]")

static_assert(cppargs::parameter_type<int>);
static_assert(cppargs::parameter_type<std::string>);
static_assert(cppargs::parameter_type<std::vector<std::string>>);
static_assert(!cppargs::parameter_type<std::vector<std::vector<std::string>>>);
static_assert(!cppargs::parameter_type<double>);

TEST("help string generation")
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

TEST("parse simple flag")
{
    cppargs::Parameters      parameters;
    auto const               help_flag    = parameters.add("help");
    auto const               version_flag = parameters.add("version");
    char const* const        command_line[] { "cppargstest", "--version" };
    cppargs::Arguments const arguments = cppargs::parse(command_line, parameters);
    REQUIRE(arguments.argv_0() == "cppargstest");
    REQUIRE(arguments[version_flag].has_value());
    REQUIRE_FALSE(arguments[help_flag].has_value());
}

TEST("parse unrecognized option")
{
    char const* const command_line[] { "cppargstest", "--version" };
    try {
        (void)cppargs::parse(command_line, {});
        REQUIRE_UNREACHABLE;
    }
    catch (cppargs::Exception const& exception) {
        REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::unrecognized_option);
        REQUIRE(exception.info().command_line == "cppargstest --version");
        REQUIRE(exception.info().error_column == 15);
        REQUIRE(exception.info().error_width == 7);
    }
}

TEST("parse missing argument")
{
    cppargs::Parameters parameters;
    auto const          interesting_option = parameters.add<int>("interesting");
    char const* const   command_line[] { "cppargstest", "--interesting" };
    try {
        (void)cppargs::parse(command_line, parameters);
        REQUIRE_UNREACHABLE;
    }
    catch (cppargs::Exception const& exception) {
        REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::missing_argument);
        REQUIRE(exception.info().command_line == "cppargstest --interesting");
        REQUIRE(exception.info().error_column == 15);
        REQUIRE(exception.info().error_width == 11);
    }
}

TEST("parse option with argument")
{
    cppargs::Parameters parameters;
    auto const          interesting_option = parameters.add<int>("interesting");
    char const* const   command_line[] { "cppargstest", "--interesting", "500" };
    auto const          arguments = cppargs::parse(command_line, parameters);
    REQUIRE(arguments[interesting_option] == "500");
}
