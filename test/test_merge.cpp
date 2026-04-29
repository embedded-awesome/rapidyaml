#ifndef RYML_SINGLE_HEADER
#include <c4/yml/std/std.hpp>
#include <c4/yml/yml.hpp>
#endif
#include <gtest/gtest.h>

#include "./test_lib/test_case.hpp"

namespace c4 {
namespace yml {

// The other test executables are written to contain the declarative-style
// YmlTestCases. This executable does not have any but the build setup
// assumes it does, and links with the test lib, which requires an existing
// get_case() function. So this is here to act as placeholder until (if?)
// proper test cases are added here. This was detected in #47 (thanks
// @cburgard).
Case const* get_case(csubstr)
{
    return nullptr;
}


void test_merge(std::initializer_list<csubstr> li, csubstr expected)
{
    Tree loaded, merged, ref;

    parse_in_arena(expected, &ref);

    // make sure the arena in the loaded tree is never resized
    size_t arena_dim = 2;
    for(csubstr src : li)
    {
        arena_dim += src.len;
    }
    loaded.reserve_arena(arena_dim);

    for(csubstr src : li)
    {
        loaded.clear(); // do not clear the arena of the loaded tree
        parse_in_arena(src, &loaded);
        _c4dbg_tree("loaded", loaded);
        merged.merge_with(&loaded);
        _c4dbg_tree("merged", merged);
    }

    _c4dbg_tree("ref", ref);

    test_compare(merged, ref);
    std::string buf_result = emitrs_yaml<std::string>(merged);
    std::string buf_expected = emitrs_yaml<std::string>(ref);

    EXPECT_EQ(buf_result, buf_expected);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST(merge, basic)
{
    test_merge(
        {
            "{a: 0, b: 1}",
            "{a: 1,     c: 20}"
        },
        "{a: 1, b: 1, c: 20}"
    );
}

TEST(merge, basic_inv)
{
    test_merge(
        {
            "{a: 1,     c: 20}",
            "{a: 0, b: 1}"
        },
        "{a: 0, c: 20, b: 1}"
    );
}

TEST(merge, dst_scalar_keeps_style)
{
    Tree dst = parse_in_arena("a: b");
    const Tree src = parse_in_arena("'a': 'c'");
    EXPECT_EQ(dst["a"], "b");
    EXPECT_EQ(src["a"], "c");
    EXPECT_TRUE(dst["a"].type().is_key_plain());
    EXPECT_TRUE(dst["a"].type().is_val_plain());
    EXPECT_TRUE(src["a"].type().is_key_squo());
    EXPECT_TRUE(src["a"].type().is_val_squo());
    _c4dbg_tree("src", src);
    _c4dbg_tree("dst", dst);
    dst.merge_with(&src);
    _c4dbg_tree("merged", dst);
    EXPECT_EQ(dst["a"], "c");
    EXPECT_EQ(src["a"], "c");
    EXPECT_TRUE(dst["a"].type().is_key_plain());
    EXPECT_TRUE(dst["a"].type().is_val_plain());
}
TEST(merge, src_scalar_assigns_style_0)
{
    test_merge(
        {
            "{}",
            "{a: 'b'}",
        },
        "{a: 'b'}"
    );
}
TEST(merge, src_scalar_assigns_style_1)
{
    test_merge(
        {
            "{}",
            "{a: 'b'}",
            "{a: \"c\"}",
        },
        "{a: 'c'}"
    );
}
TEST(merge, src_scalar_assigns_style_2)
{
    test_merge(
        {
            "{}",
            "{a: 'b'}",
            "{a: \"c\"}",
            "{a: d}",
        },
        "{a: 'd'}"
    );
}
TEST(merge, src_map_assigns_style_0)
{
    test_merge(
        {
            "{}",
            "a: 'b'",
            "{aa: \"bb\"}",
        },
        "{a: 'b', aa: \"bb\"}"
        );
}
TEST(merge, src_map_assigns_style_1)
{
    test_merge(
        {
            "foo: bar",
            "{a: 'b'}",
            "{foo: \"bar\"}",
        },
        "foo: bar\na: 'b'\n"
        );
}
TEST(merge, src_seq_assigns_style_0)
{
    test_merge(
        {
            "[]",
            "- 0\n- 1\n",
            "- 2\n- 3\n",
        },
        "[0,1,2,3]"
        );
}
TEST(merge, src_seq_assigns_style_1)
{
    test_merge(
        {
            "- 0\n- 1\n",
            "[a,b]",
            "[c,d]",
        },
        "- 0\n- 1\n- a\n- b\n- c\n- d\n"
        );
}

TEST(merge, val_to_seq)
{
    test_merge(
        {
            "{a: 0, b: 1}",
            "{a: [1, 2]}"
        },
        "{a: [1, 2], b: 1}"
    );
}

TEST(merge, seq_to_val)
{
    test_merge(
        {
            "{a: [1, 2]}",
            "{a: 0, b: 1}",
        },
        "{a: 0, b: 1}"
    );
}

TEST(merge, val_to_map)
{
    test_merge(
        {
            "{a: 0, b: 1}",
            "{a: {c: 10, d: 20}}"
        },
        "{a: {c: 10, d: 20}, b: 1}"
    );
}

TEST(merge, map_to_val)
{
    test_merge(
        {
            "{a: {c: 10, d: 20}}",
            "{a: 0, b: 1}",
        },
        "{a: 0, b: 1}"
    );
}

TEST(merge, seq_no_overlap)
{
    test_merge(
        {"[0, 1, 2]", "[3, 4, 5]", "[6, 7, 8]"},
        "[0, 1, 2, 3, 4, 5, 6, 7, 8]"
    );
}



TEST(merge, seq_overlap)
{
    test_merge(
        {"[0, 1, 2]", "[1, 2, 3]", "[2, 3, 4]"},
        "[0, 1, 2, 1, 2, 3, 2, 3, 4]"
        // or this? "[0, 1, 2, 3, 4]"
    );
}



TEST(merge, map_orthogonal)
{
    test_merge(
        {
            "{a: 0}",
            "{b: 1}",
            "{c: 2}"
        },
        "{a: 0, b: 1, c: 2}"
    );
}


TEST(merge, map_overriding)
{
    test_merge(
        {
            "{a: 0}",
            "{a: 1, b: 1}",
            "{c: 2}"
        },
        "{a: 1, b: 1, c: 2}"
    );
}

TEST(merge, map_overriding_multiple)
{
    test_merge(
        {
            "{a: 0}",
            "{a: 1, b: 1}",
            "{c: 2}",
            "{a: 2}",
            "{a: 3}",
            "{c: 4}",
            "{c: 5}",
            "{a: 4}",
        },
        "{a: 4, b: 1, c: 5}"
    );
}


TEST(merge, seq_nested_in_map)
{
    test_merge(
        {
            "{a: 0, seq: [a, b, c], d: 2}",
            "{a: 1, seq: [d, e, f], d: 3, c: 3}"
        },
        "{a: 1, seq: [a, b, c, d, e, f], d: 3, c: 3}"
    );
}


TEST(merge, seq_nested_in_map_override_with_map)
{
    test_merge(
        {
            "{a: 0, ovr: [a, b, c], d: 2}",
            "{a: 1, ovr: {d: 0, b: 1, c: 2}, d: 3, c: 3}"
        },
        "{a: 1, ovr: {d: 0, b: 1, c: 2}, d: 3, c: 3}"
    );
}


TEST(merge, seq_nested_in_map_override_with_keyval)
{
    test_merge(
        {
            "{a: 0, ovr: [a, b, c], d: 2}",
            "{a: 1, ovr: foo, d: 3, c: 3}"
        },
        "{a: 1, ovr: foo, d: 3, c: 3}"
    );
}


//-----------------------------------------------------------------------------
// Tests for NodeRef::merge() — cross-tree merging with arena copy
//-----------------------------------------------------------------------------

// Helper: parse yaml into a tree and return a NodeRef to its root.
// The tree is owned by the caller.
static void check_no_src_arena_refs(Tree const& dst, Tree const& src)
{
    csubstr src_arena = src.arena();
    // Walk all nodes in dst and assert none of their scalars point into src's arena
    for(id_type i = 0; i < dst.capacity(); ++i)
    {
        NodeData const* n = dst.get(i);
        if(!n) continue;
        if(n->m_type == NOTYPE) continue;
        if(!src_arena.empty())
        {
            EXPECT_FALSE(src_arena.is_super(n->m_key.scalar))  << "key scalar dangling into src arena";
            EXPECT_FALSE(src_arena.is_super(n->m_key.tag))     << "key tag dangling into src arena";
            EXPECT_FALSE(src_arena.is_super(n->m_key.anchor))  << "key anchor dangling into src arena";
            EXPECT_FALSE(src_arena.is_super(n->m_val.scalar))  << "val scalar dangling into src arena";
            EXPECT_FALSE(src_arena.is_super(n->m_val.tag))     << "val tag dangling into src arena";
            EXPECT_FALSE(src_arena.is_super(n->m_val.anchor))  << "val anchor dangling into src arena";
        }
    }
}

TEST(NodeRef_merge, map_from_another_tree)
{
    Tree dst = parse_in_arena("{a: 0, b: 1}");
    Tree src = parse_in_arena("{a: 1, c: 20}");
    dst.rootref().merge(src.rootref());
    EXPECT_EQ(dst["a"].val(), "1");
    EXPECT_EQ(dst["b"].val(), "1");
    EXPECT_EQ(dst["c"].val(), "20");
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, seq_from_another_tree)
{
    Tree dst = parse_in_arena("[0, 1, 2]");
    Tree src = parse_in_arena("[3, 4, 5]");
    dst.rootref().merge(src.rootref());
    ASSERT_EQ(dst.rootref().num_children(), 6u);
    EXPECT_EQ(dst[0].val(), "0");
    EXPECT_EQ(dst[3].val(), "3");
    EXPECT_EQ(dst[5].val(), "5");
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, nested_from_another_tree)
{
    Tree dst = parse_in_arena("{a: {x: 1}, b: [10, 20]}");
    Tree src = parse_in_arena("{a: {x: 99, y: 2}, b: [30]}");
    dst.rootref().merge(src.rootref());
    EXPECT_EQ(dst["a"]["x"].val(), "99");
    EXPECT_EQ(dst["a"]["y"].val(), "2");
    ASSERT_EQ(dst["b"].num_children(), 3u);
    EXPECT_EQ(dst["b"][0].val(), "10");
    EXPECT_EQ(dst["b"][2].val(), "30");
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, src_destroyed_after_merge)
{
    Tree dst = parse_in_arena("{a: 0}");
    {
        Tree src = parse_in_arena("{a: hello, b: world}");
        dst.rootref().merge(src.rootref());
        // src goes out of scope and its arena is freed here
    }
    // Values must still be valid — they live in dst's arena
    EXPECT_EQ(dst["a"].val(), "hello");
    EXPECT_EQ(dst["b"].val(), "world");
}

TEST(NodeRef_merge, same_tree_self_merge_is_noop)
{
    Tree t = parse_in_arena("{a: 0, b: 1}");
    NodeRef root = t.rootref();
    root.merge(t.rootref());  // must not crash or corrupt the tree
    EXPECT_EQ(t["a"].val(), "0");
    EXPECT_EQ(t["b"].val(), "1");
    EXPECT_EQ(t.rootref().num_children(), 2u);
}

TEST(NodeRef_merge, style_preserved_for_existing_dst_key)
{
    Tree dst = parse_in_arena("a: b");
    Tree src = parse_in_arena("'a': 'new_val'");
    EXPECT_TRUE(dst["a"].type().is_key_plain());
    EXPECT_TRUE(dst["a"].type().is_val_plain());
    dst.rootref().merge(src.rootref());
    EXPECT_EQ(dst["a"].val(), "new_val");
    // dst had plain style → style is preserved for the existing key
    EXPECT_TRUE(dst["a"].type().is_key_plain());
    EXPECT_TRUE(dst["a"].type().is_val_plain());
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, style_copied_for_new_key)
{
    Tree dst = parse_in_arena("{}");
    Tree src = parse_in_arena("{a: 'b'}");
    dst.rootref().merge(src.rootref());
    EXPECT_EQ(dst["a"].val(), "b");
    // key was absent in dst → style from src is used
    EXPECT_TRUE(dst["a"].type().is_val_squo());
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, val_overrides_map_in_dst)
{
    Tree dst = parse_in_arena("{a: {x: 1}}");
    Tree src = parse_in_arena("{a: scalar}");
    dst.rootref().merge(src.rootref());
    EXPECT_TRUE(dst["a"].is_keyval());
    EXPECT_EQ(dst["a"].val(), "scalar");
    check_no_src_arena_refs(dst, src);
}

TEST(NodeRef_merge, map_overrides_val_in_dst)
{
    Tree dst = parse_in_arena("{a: scalar}");
    Tree src = parse_in_arena("{a: {x: 1}}");
    dst.rootref().merge(src.rootref());
    EXPECT_TRUE(dst["a"].is_map());
    EXPECT_EQ(dst["a"]["x"].val(), "1");
    check_no_src_arena_refs(dst, src);
}

} // namespace yml
} // namespace c4
