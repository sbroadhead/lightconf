What is lightconf?
------
lightconf is a lightweight header-only configuration file library for C++11. It is designed to be (almost) compatible with JSON.

### Features
- Header-only means it can be copied directly into your project and used without introducing any deployment issues.
- Hierarchical configuration files with a simple path-based interface for accessing nested properties.
- The primary lightconf text format `.config` is structurally identical to JSON but supports comments. Comments, blank lines, and ordering will be preserved when updating an existing file.
- It can also read and write JSON directly.
- Easily extensible to allow serializing custom types and enums automatically.
- BSD licensed.

Certain features used in lightconf, such as `decltype` and variadic templates, mean that it is only usable by compilers that support C++11. It has been tested using Clang 3.2 and GCC 4.7.2.

### Alternatives
There are various common ways to implement configuration files:
- _JSON_ is a common text format for storing structured data. The drawback of using a JSON library directly is that most JSON libraries do not abstract away the physical structure of a JSON document, and still require explicitly drilling down and extracting values from the document, especially when dealing with custom types. lightconf is not a replacement for JSON, but rather a more specialized interface that is capable of reading and writing both JSON and a slightly more human-friendly format called `.config`.
- _XML_ suffers from the same problems as using a JSON library with the added problem of being extremely verbose.
- _Boost.PropertyTree_ is a nice library similar to lightconf that supports JSON among other formats. The primary drawback is its dependency on Boost.
- _INI_ is a very simple flat key/value format. It does not support hierarchical or vector data.

### File Format
The text-based file format that lightconf is designed to use for primary serialization and deserialization is referred to internally as `.config` (simply to differentiate it from JSON, the file extension is not important). It differs from JSON in the following ways:

- Keys are bare identifiers rather than strings, and are of the form `[A-Za-z][A-Za-z0-9_\-]*` (if a JSON file has keys which don't match this pattern, an exception will be thrown when trying to deserialize it)
- Keys are separated from values by an `=` rather than a colon
- Commas (including trailing commas) in lists and groups are optional
- Comments are supported (JSON with comments will read as well, but the comments will not be preserved when saving a JSON file)
- JSON supports documents with arrays as root elements, but `.config` files always have a group (an object in JSON) as a root, with no braces

### Quick Tutorial
A full working example is available in `sample/lightconf_sample.cpp`. This is a very quick example of how to use lightconf.

The primary object containing configuration information is `lightconf::group`. It acts like a map from string keys to `lightconf::value` values, and is analogous to JSON's _objects_. A value is effectively a union type which can contain the standard JSON types: doubles, strings, bools, arrays (implemented as `std::vector<lightconf::value>`) and other groups.

Values are accessed via paths, which are period separated names corresponding to nested groups. The path `foo.bar.baz` corresponds to the value 123 in the configuration layout `foo = { bar = { baz = 123 } }`.

```cpp
// Deserialize a configuration file stored in a string
lightconf::group config_group = lightconf::config_format::read(source);

// Retrieve a value from the configuration group (throwing an exception if it doesn't exist)
int foo_count = config_group.get<int>("counts.foo_count");

// Retrieve a value from the configuration group (returning a default value if it doesn't exist)
bool is_bar = config_group.get<bool>("flags.is_bar", false);

// Set a value in the configuration group, creating parent groups
config_group.set<double>("constants.transcendental.pi", 3.14159);

// Typed vectors and tuples can also be stored and retrieved, as long as the inner types can be stored
config_group.set<std::vector<string>>("names", { "Fred", "George", "Stephen" });
std::vector<string> names = config_group.get<std::vector<string>>("names");
config_group.set<std::tuple<int, bool>>("my_tuple", std::make_tuple<int, bool>(10, false));
std::tuple<int, bool> my_tuple = config_group.get<std::tuple<int, bool>>("my_tuple");

// Generate a new configuration file using the original one as a base, and attempting to keep 
// lines under 80 characters long by wrapping lists and groups that exceed that length
std::string new_source = lightconf::config_format::write(config_group, source, 80);

// We can also create a JSON version
std::string json_source = lightconf::json_format::write(config_group);
```

Custom types and enums can also be serialized directly by adding specializations of the template class `lightconf::value_type_info`. See `sample/lightconf_sample.cpp` for an example.
