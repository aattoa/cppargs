# cppargs
A simple library for handling command line arguments in C++20 and up.

# Why cppargs?
Most existing solutions, while effective, have some suboptimal properties:

- They require the user to spell each option name *as a string* more than once:
  first when declaring the option, and again for every access. This makes them
  somewhat error-prone, as changing the string in one place but forgetting to
  do so elsewhere does not render the program ill-formed.
- They require the user to write some kind of manual cast to access the
  argument value, usually spelled `option.as<T>()`. This is error-prone for the
  same reason as having to write the option name string multiple times. C++ is
  a statically typed language, so why not take advantage of that?

`cppargs` solves both of these issues by only allowing the user to refer to
arguments through tag objects that keep track of their type and identity.

# Example usage

```C++
#include <cppargs.hpp>
#include <exception>
#include <print>

int main(int argc, char const** argv)
{
    try {
        cppargs::Parameters parameters;

        auto const help   = parameters.add('h', "help", "Show this help text");
        auto const square = parameters.add<int>('s', "square", "Square an integer");

        auto const arguments = cppargs::parse(argc, argv, parameters);

        if (arguments[help]) {
            std::println("List of options:\n{}", parameters.help_string());
            return 0;
        }
        if (auto const integer = arguments[square]) {
            // integer is of type std::optional<int>
            int const x = integer.value();
            std::println("The square of {} is {}", x, x*x);
        }
    }
    catch (std::exception const& exception) {
        std::println(stderr, "Error: {}", exception.what());
    }
}
```

The above program might be invoked as follows:

```Shell
$ prog --help
List of options:
    --help, -h         : Show this help text
    --square, -s [arg] : Square an integer

$ prog --square
Error: Missing argument for parameter 'square'

$ prog --square hello
Error: Invalid argument 'hello'

$ prog --square -4
The square of -4 is 16

$ prog -s50
The square of 50 is 2500
```

# Extensibility

`cppargs` supports arguments of any type, as long as the user has specified how
the given type `T` should be parsed, which can be done by specializing
`cppargs::Argument<T>`. The specialization should have a member function with
the signature `static auto parse(std::string_view) -> std::optional<T>`.

```C++
// Arbitrary user-defined type
enum class Color { red, green, blue };

// Tell cppargs how to handle it
template <>
struct cppargs::Argument<Color> {
    static auto parse(std::string_view const view) -> std::optional<Color>
    {
        if (view == "red")   return Color::red;
        if (view == "green") return Color::green;
        if (view == "blue")  return Color::blue;
        return std::nullopt;
    }
};
```
