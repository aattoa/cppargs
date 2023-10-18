#include <cppargs.hpp>
#include <catch2/catch_test_macros.hpp>

#define REQUIRE_UNREACHABLE REQUIRE(false)
#define TEST(name)          TEST_CASE("cppargs: " name, "[cppargs]")

using namespace std::literals;

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

TEST("parse valid simple flag")
{
    cppargs::Parameters parameters;
    auto const          help_flag    = parameters.add('h', "help");
    auto const          version_flag = parameters.add('v', "version");
    SECTION("long name")
    {
        char const* const command_line[] { "cppargstest", "--version" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE(arguments.argv_0() == "cppargstest");
        REQUIRE(arguments[version_flag].has_value());
        REQUIRE_FALSE(arguments[help_flag].has_value());
    }
    SECTION("short name")
    {
        char const* const command_line[] { "cppargstest", "-h" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE(arguments.argv_0() == "cppargstest");
        REQUIRE(arguments[help_flag].has_value());
        REQUIRE_FALSE(arguments[version_flag].has_value());
    }
}

TEST("parse invalid simple flag")
{
    cppargs::Parameters parameters;
    auto const          help_flag = parameters.add('h', "help");
    SECTION("long name")
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
            REQUIRE(exception.what() == "Error: Unrecognized option 'version'"sv);
        }
    }
    SECTION("short name")
    {
        char const* const command_line[] { "cppargstest", "-v" };
        try {
            (void)cppargs::parse(command_line, {});
            REQUIRE_UNREACHABLE;
        }
        catch (cppargs::Exception const& exception) {
            REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::unrecognized_option);
            REQUIRE(exception.info().command_line == "cppargstest -v");
            REQUIRE(exception.info().error_column == 14);
            REQUIRE(exception.info().error_width == 1);
            REQUIRE(exception.what() == "Error: Unrecognized option 'v'"sv);
        }
    }
}

TEST("parse aggregate short names")
{
    cppargs::Parameters parameters;
    auto const          help_flag    = parameters.add('h', "help");
    auto const          version_flag = parameters.add('v', "version");
    SECTION("valid")
    {
        char const* const command_line[] { "cppargstest", "-vh" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE(arguments.argv_0() == "cppargstest");
        REQUIRE(arguments[help_flag].has_value());
        REQUIRE(arguments[version_flag].has_value());
    }
    SECTION("invalid")
    {
        char const* const command_line[] { "cppargstest", "-vx" };
        try {
            (void)cppargs::parse(command_line, parameters);
            REQUIRE_UNREACHABLE;
        }
        catch (cppargs::Exception const& exception) {
            REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::unrecognized_option);
            REQUIRE(exception.info().command_line == "cppargstest -vx");
            REQUIRE(exception.info().error_column == 15);
            REQUIRE(exception.info().error_width == 1);
            REQUIRE(exception.what() == "Error: Unrecognized option 'x'"sv);
        }
    }
}

TEST("parse named argument")
{
    cppargs::Parameters parameters;
    auto const          a = parameters.add<int>('a', "aaa");
    auto const          b = parameters.add<std::string_view>('b', "bbb");
    auto const          c = parameters.add('c', "ccc");
    SECTION("long name")
    {
        char const* const command_line[] { "cppargstest", "--aaa", "53" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE(arguments[a] == 53);
        REQUIRE_FALSE(arguments[b].has_value());
        REQUIRE_FALSE(arguments[c].has_value());
    }
    SECTION("short name")
    {
        char const* const command_line[] { "cppargstest", "-a56", "-b", "qwerty" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE(arguments[a] == 56);
        REQUIRE(arguments[b] == "qwerty");
        REQUIRE_FALSE(arguments[c].has_value());
    }
    SECTION("aggregate short names")
    {
        char const* const command_line[] { "cppargstest", "-cbabc" };
        auto const        arguments = cppargs::parse(command_line, parameters);
        REQUIRE_FALSE(arguments[a].has_value());
        REQUIRE(arguments[b] == "abc");
        REQUIRE(arguments[c].has_value());
    }
}

TEST("parse boolean argument")
{
    auto const parse = cppargs::Argument<bool>::parse;

    REQUIRE(parse("true") == true);
    REQUIRE(parse("yes") == true);
    REQUIRE(parse("on") == true);
    REQUIRE(parse("1") == true);

    REQUIRE(parse("false") == false);
    REQUIRE(parse("off") == false);
    REQUIRE(parse("no") == false);
    REQUIRE(parse("0") == false);

    REQUIRE_FALSE(parse("truew").has_value());
    REQUIRE_FALSE(parse("truew").has_value());
    REQUIRE_FALSE(parse("falsew").has_value());
    REQUIRE_FALSE(parse("offf").has_value());
    REQUIRE_FALSE(parse("5").has_value());
}

TEST("parse integral argument")
{
    SECTION("signed")
    {
        auto const parse = cppargs::Argument<int>::parse;
        REQUIRE(parse("53") == 53);
        REQUIRE(parse("-53") == -53);
        REQUIRE_FALSE(parse("+53").has_value());
    }
    SECTION("unsigned")
    {
        auto const parse = cppargs::Argument<unsigned>::parse;
        REQUIRE(parse("53") == 53);
        REQUIRE_FALSE(parse("-53").has_value());
        REQUIRE_FALSE(parse("+53").has_value());
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
        REQUIRE(exception.what() == "Error: Missing argument for parameter 'interesting'"sv);
    }
}

TEST("parse invalid argument")
{
    cppargs::Parameters parameters;
    auto const          interesting_option = parameters.add<int>("interesting");
    char const* const   command_line[] { "cppargstest", "--interesting", "hello" };
    try {
        (void)cppargs::parse(command_line, parameters);
        REQUIRE_UNREACHABLE;
    }
    catch (cppargs::Exception const& exception) {
        REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::invalid_argument);
        REQUIRE(exception.info().command_line == "cppargstest --interesting hello");
        REQUIRE(exception.info().error_column == 27);
        REQUIRE(exception.info().error_width == 5);
        REQUIRE(exception.what() == "Error: Invalid argument 'hello'"sv);
    }
}

TEST("parse positional argument")
{
    cppargs::Parameters parameters;
    auto const          help_flag = parameters.add("help");
    char const* const   command_line[] { "cppargstest", "hello" };
    try {
        (void)cppargs::parse(command_line, parameters);
        REQUIRE_UNREACHABLE;
    }
    catch (cppargs::Exception const& exception) {
        REQUIRE(exception.info().kind == cppargs::Parse_error_info::Kind::positional_argument);
        REQUIRE(exception.info().command_line == "cppargstest hello");
        REQUIRE(exception.info().error_column == 13);
        REQUIRE(exception.info().error_width == 5);
        REQUIRE(exception.what() == "Error: Positional arguments are not supported yet: 'hello'"sv);
    }
}
