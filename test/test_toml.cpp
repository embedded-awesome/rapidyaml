#ifndef RYML_SINGLE_HEADER
#include "c4/yml/std/std.hpp"
#include "c4/yml/parse_toml.hpp"
#include "c4/yml/emit.hpp"
#include <c4/format.hpp>
#include <c4/yml/detail/checks.hpp>
#include <c4/yml/detail/print.hpp>
#endif

#include "./test_lib/test_case.hpp"

#include <gtest/gtest.h>

namespace c4 {
namespace yml {

//-----------------------------------------------------------------------------
// Basic TOML parsing tests

TEST(parse_toml, basic_string)
{
    csubstr toml = R"(
name = "Tom"
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t.rootref().has_child("name"));
    EXPECT_EQ(t["name"].val(), "Tom");
}

TEST(parse_toml, basic_integer)
{
    csubstr toml = R"(
age = 42
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t.rootref().has_child("age"));
    EXPECT_EQ(t["age"].val(), "42");
}

TEST(parse_toml, basic_float)
{
    csubstr toml = R"(
pi = 3.14159
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t.rootref().has_child("pi"));
    // Float formatting may vary, so just check it's parseable
    EXPECT_TRUE(t["pi"].val().begins_with("3.14"));
}

TEST(parse_toml, basic_boolean)
{
    csubstr toml = R"(
enabled = true
disabled = false
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_EQ(t["enabled"].val(), "true");
    EXPECT_EQ(t["disabled"].val(), "false");
}

TEST(parse_toml, basic_array)
{
    csubstr toml = R"(
numbers = [1, 2, 3]
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t["numbers"].is_seq());
    EXPECT_EQ(t["numbers"].num_children(), 3u);
    EXPECT_EQ(t["numbers"][0].val(), "1");
    EXPECT_EQ(t["numbers"][1].val(), "2");
    EXPECT_EQ(t["numbers"][2].val(), "3");
}

TEST(parse_toml, basic_table)
{
    csubstr toml = R"(
[database]
server = "192.168.1.1"
port = 5432
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t["database"].is_map());
    EXPECT_EQ(t["database"]["server"].val(), "192.168.1.1");
    EXPECT_EQ(t["database"]["port"].val(), "5432");
}

TEST(parse_toml, nested_table)
{
    csubstr toml = R"(
[servers]

[servers.alpha]
ip = "10.0.0.1"

[servers.beta]
ip = "10.0.0.2"
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t["servers"].is_map());
    EXPECT_TRUE(t["servers"]["alpha"].is_map());
    EXPECT_TRUE(t["servers"]["beta"].is_map());
    EXPECT_EQ(t["servers"]["alpha"]["ip"].val(), "10.0.0.1");
    EXPECT_EQ(t["servers"]["beta"]["ip"].val(), "10.0.0.2");
}

TEST(parse_toml, inline_table)
{
    csubstr toml = R"(
point = { x = 1, y = 2 }
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t["point"].is_map());
    EXPECT_EQ(t["point"]["x"].val(), "1");
    EXPECT_EQ(t["point"]["y"].val(), "2");
}

TEST(parse_toml, array_of_tables)
{
    csubstr toml = R"(
[[products]]
name = "Hammer"
sku = 738594937

[[products]]
name = "Nail"
sku = 284758393
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t["products"].is_seq());
    EXPECT_EQ(t["products"].num_children(), 2u);
    EXPECT_EQ(t["products"][0]["name"].val(), "Hammer");
    EXPECT_EQ(t["products"][1]["name"].val(), "Nail");
}

TEST(parse_toml, datetime)
{
    csubstr toml = R"(
created = 2023-05-27T10:30:00Z
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t.rootref().has_child("created"));
    // Just check that it parsed
    EXPECT_FALSE(t["created"].val().empty());
}

TEST(parse_toml, multiline_string)
{
    csubstr toml = R"(
description = """
This is a
multiline string.
""")";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_TRUE(t.rootref().has_child("description"));
    // The multiline string should be preserved
    EXPECT_TRUE(t["description"].val().find("multiline") != csubstr::npos);
}

TEST(parse_toml, special_floats)
{
    csubstr toml = R"(
positive_inf = inf
negative_inf = -inf
not_a_number = nan
)";
    Tree t = parse_toml_in_arena(toml);
    EXPECT_TRUE(t.rootref().is_map());
    // TOML++ converts these to the YAML equivalents
    EXPECT_EQ(t["positive_inf"].val(), ".inf");
    EXPECT_EQ(t["negative_inf"].val(), "-.inf");
    EXPECT_EQ(t["not_a_number"].val(), ".nan");
}

TEST(parse_toml, complex_example)
{
    csubstr toml = R"(
[package]
name = "myapp"
version = "1.0.0"
authors = ["Alice", "Bob"]

[dependencies]
serde = "1.0"
tokio = { version = "1.0", features = ["full"] }

[[bin]]
name = "main"
path = "src/main.rs"
)";
    Tree t = parse_toml_in_arena(toml);

    // Check package section
    EXPECT_TRUE(t["package"].is_map());
    EXPECT_EQ(t["package"]["name"].val(), "myapp");
    EXPECT_EQ(t["package"]["version"].val(), "1.0.0");
    EXPECT_TRUE(t["package"]["authors"].is_seq());
    EXPECT_EQ(t["package"]["authors"][0].val(), "Alice");
    EXPECT_EQ(t["package"]["authors"][1].val(), "Bob");

    // Check dependencies section
    EXPECT_TRUE(t["dependencies"].is_map());
    EXPECT_EQ(t["dependencies"]["serde"].val(), "1.0");
    EXPECT_TRUE(t["dependencies"]["tokio"].is_map());
    EXPECT_EQ(t["dependencies"]["tokio"]["version"].val(), "1.0");

    // Check bin array
    EXPECT_TRUE(t["bin"].is_seq());
    EXPECT_EQ(t["bin"][0]["name"].val(), "main");
}

//-----------------------------------------------------------------------------
// Test parse_toml_in_place overloads

TEST(parse_toml_in_place, into_existing_tree)
{
    csubstr toml = R"(key = "value")";
    Tree t;
    parse_toml_in_place(toml, &t);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_EQ(t["key"].val(), "value");
}

TEST(parse_toml_in_place, with_filename)
{
    csubstr toml = R"(key = "value")";
    Tree t = parse_toml_in_place("test.toml", toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_EQ(t["key"].val(), "value");
}

//-----------------------------------------------------------------------------
// Test parse_toml_in_arena overloads

TEST(parse_toml_in_arena, into_existing_tree)
{
    csubstr toml = R"(key = "value")";
    Tree t;
    parse_toml_in_arena(toml, &t);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_EQ(t["key"].val(), "value");
}

TEST(parse_toml_in_arena, with_filename)
{
    csubstr toml = R"(key = "value")";
    Tree t = parse_toml_in_arena("test.toml", toml);
    EXPECT_TRUE(t.rootref().is_map());
    EXPECT_EQ(t["key"].val(), "value");
}

//-----------------------------------------------------------------------------
// Test emit to YAML after parsing TOML

TEST(parse_toml, emit_yaml)
{
    csubstr toml = R"(
[server]
host = "localhost"
port = 8080
)";
    Tree t = parse_toml_in_arena(toml);

    std::string yaml;
    emitrs_yaml(t, &yaml);

    // Parse the YAML back
    Tree t2 = parse_in_arena(to_csubstr(yaml));

    EXPECT_EQ(t2["server"]["host"].val(), "localhost");
    EXPECT_EQ(t2["server"]["port"].val(), "8080");
}

TEST(parse_toml, emit_json)
{
    csubstr toml = R"(
name = "test"
count = 42
)";
    Tree t = parse_toml_in_arena(toml);

    std::string json;
    emitrs_json(t, &json);

    // Parse the JSON back
    Tree t2 = parse_json_in_arena(to_csubstr(json));

    EXPECT_EQ(t2["name"].val(), "test");
    EXPECT_EQ(t2["count"].val(), "42");
}

//-----------------------------------------------------------------------------
// this is needed to use the test case library
Case const* get_case(csubstr /*name*/)
{
    return nullptr;
}

} // namespace yml
} // namespace c4
