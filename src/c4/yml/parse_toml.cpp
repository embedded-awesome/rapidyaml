#include "c4/yml/parse_toml.hpp"
#include "c4/yml/node.hpp"
#include "c4/yml/error.hpp"

#include <toml++/toml.hpp>

#include <sstream>
#include <cmath>

namespace c4 {
namespace yml {

namespace {

// Forward declaration
void convert_toml_value(const toml::node& toml_node, Tree* tree, id_type node_id);

// Helper to copy a string to the tree's arena and return a csubstr
csubstr to_arena(Tree* tree, std::string_view sv)
{
    if (sv.empty())
        return csubstr{};
    substr buf = tree->alloc_arena(sv.size());
    memcpy(buf.str, sv.data(), sv.size());
    return buf;
}

// Helper to convert a scalar value to string
template<typename T>
csubstr scalar_to_arena(Tree* tree, const T& value)
{
    std::ostringstream ss;
    ss << value;
    std::string str = ss.str();
    return to_arena(tree, str);
}

// Helper to handle TOML parse errors
[[noreturn]] void handle_toml_parse_error(const toml::parse_error& err)
{
    std::ostringstream ss;
    ss << err;
    std::string msg = ss.str();
    // Use the global callbacks error handler
    Callbacks const& cb = get_callbacks();
    ErrorDataBasic errdata = {};
    cb.m_error_basic(csubstr(msg.c_str(), msg.size()), errdata, cb.m_user_data);
    // This function should never return, but if the error handler somehow does, abort
    std::abort();
}

// Helper to set a scalar value on a node
void set_scalar_value(Tree* tree, id_type node_id, csubstr val, NodeType_e extra_flags = NOTYPE)
{
    if (tree->has_key(node_id))
    {
        csubstr key = tree->key(node_id);
        tree->to_keyval(node_id, key, val);
    }
    else
    {
        tree->to_val(node_id, val);
    }
    if (extra_flags != NOTYPE)
    {
        tree->_add_flags(node_id, extra_flags);
    }
}

// Convert any TOML value to tree nodes
void convert_toml_value(const toml::node& toml_node, Tree* tree, id_type node_id)
{
    if (toml_node.is_table())
    {
        const auto& tbl = *toml_node.as_table();
        // If this node already has a key, preserve it
        if (tree->has_key(node_id))
        {
            csubstr key = tree->key(node_id);
            tree->to_map(node_id, key);
        }
        else
        {
            tree->to_map(node_id);
        }
        for (const auto& [key, value] : tbl)
        {
            id_type child_id = tree->append_child(node_id);
            tree->to_keyval(child_id, to_arena(tree, std::string_view(key)), csubstr{});
            convert_toml_value(value, tree, child_id);
        }
    }
    else if (toml_node.is_array())
    {
        const auto& arr = *toml_node.as_array();
        // If this node already has a key, preserve it
        if (tree->has_key(node_id))
        {
            csubstr key = tree->key(node_id);
            tree->to_seq(node_id, key);
        }
        else
        {
            tree->to_seq(node_id);
        }
        for (const auto& elem : arr)
        {
            id_type child_id = tree->append_child(node_id);
            convert_toml_value(elem, tree, child_id);
        }
    }
    else if (toml_node.is_string())
    {
        std::string_view str_val = toml_node.as_string()->get();
        csubstr val = to_arena(tree, str_val);
        set_scalar_value(tree, node_id, val, VAL_DQUO);
    }
    else if (toml_node.is_integer())
    {
        int64_t int_val = toml_node.as_integer()->get();
        csubstr val = scalar_to_arena(tree, int_val);
        set_scalar_value(tree, node_id, val);
    }
    else if (toml_node.is_floating_point())
    {
        double d = toml_node.as_floating_point()->get();
        csubstr val;
        if (std::isinf(d))
        {
            val = d > 0 ? to_arena(tree, ".inf") : to_arena(tree, "-.inf");
        }
        else if (std::isnan(d))
        {
            val = to_arena(tree, ".nan");
        }
        else
        {
            val = scalar_to_arena(tree, d);
        }
        set_scalar_value(tree, node_id, val);
    }
    else if (toml_node.is_boolean())
    {
        bool bool_val = toml_node.as_boolean()->get();
        csubstr val = bool_val ? to_arena(tree, "true") : to_arena(tree, "false");
        set_scalar_value(tree, node_id, val);
    }
    else if (toml_node.is_date())
    {
        csubstr val = scalar_to_arena(tree, toml_node.as_date()->get());
        set_scalar_value(tree, node_id, val);
    }
    else if (toml_node.is_time())
    {
        csubstr val = scalar_to_arena(tree, toml_node.as_time()->get());
        set_scalar_value(tree, node_id, val);
    }
    else if (toml_node.is_date_time())
    {
        csubstr val = scalar_to_arena(tree, toml_node.as_date_time()->get());
        set_scalar_value(tree, node_id, val);
    }
}

void parse_toml_impl(csubstr filename, csubstr toml, Tree* t, id_type node_id)
{
    std::string_view toml_sv(toml.str, toml.len);
    std::string filename_str;
    if (filename.len > 0)
        filename_str = std::string(filename.str, filename.len);

    toml::table tbl;
    try
    {
        tbl = toml::parse(toml_sv, filename_str);
    }
    catch (const toml::parse_error& err)
    {
        handle_toml_parse_error(err);
    }

    convert_toml_value(tbl, t, node_id);
}

void parse_toml_file_impl(csubstr filename, Tree* t, id_type node_id)
{
    std::string filename_str(filename.str, filename.len);

    toml::table tbl;
    try
    {
        tbl = toml::parse_file(filename_str);
    }
    catch (const toml::parse_error& err)
    {
        handle_toml_parse_error(err);
    }

    convert_toml_value(tbl, t, node_id);
}

} // namespace

//-----------------------------------------------------------------------------
// parse_toml_in_place overloads

void parse_toml_in_place(csubstr filename, csubstr toml, Tree *t, id_type node_id)
{
    parse_toml_impl(filename, toml, t, node_id);
}

void parse_toml_in_place(csubstr toml, Tree *t, id_type node_id)
{
    parse_toml_impl({}, toml, t, node_id);
}

void parse_toml_in_place(csubstr filename, csubstr toml, Tree *t)
{
    parse_toml_impl(filename, toml, t, t->root_id());
}

void parse_toml_in_place(csubstr toml, Tree *t)
{
    parse_toml_impl({}, toml, t, t->root_id());
}

void parse_toml_in_place(csubstr filename, csubstr toml, NodeRef node)
{
    parse_toml_impl(filename, toml, node.tree(), node.id());
}

void parse_toml_in_place(csubstr toml, NodeRef node)
{
    parse_toml_impl({}, toml, node.tree(), node.id());
}

Tree parse_toml_in_place(csubstr filename, csubstr toml)
{
    Tree t;
    parse_toml_impl(filename, toml, &t, t.root_id());
    return t;
}

Tree parse_toml_in_place(csubstr toml)
{
    Tree t;
    parse_toml_impl({}, toml, &t, t.root_id());
    return t;
}

//-----------------------------------------------------------------------------
// parse_toml_in_arena overloads

void parse_toml_in_arena(csubstr filename, csubstr toml, Tree *t, id_type node_id)
{
    // Copy to arena first
    substr arena_copy = t->alloc_arena(toml.len);
    memcpy(arena_copy.str, toml.str, toml.len);
    parse_toml_impl(filename, arena_copy, t, node_id);
}

void parse_toml_in_arena(csubstr toml, Tree *t, id_type node_id)
{
    parse_toml_in_arena({}, toml, t, node_id);
}

void parse_toml_in_arena(csubstr filename, csubstr toml, Tree *t)
{
    parse_toml_in_arena(filename, toml, t, t->root_id());
}

void parse_toml_in_arena(csubstr toml, Tree *t)
{
    parse_toml_in_arena({}, toml, t, t->root_id());
}

void parse_toml_in_arena(csubstr filename, csubstr toml, NodeRef node)
{
    parse_toml_in_arena(filename, toml, node.tree(), node.id());
}

void parse_toml_in_arena(csubstr toml, NodeRef node)
{
    parse_toml_in_arena({}, toml, node.tree(), node.id());
}

Tree parse_toml_in_arena(csubstr filename, csubstr toml)
{
    Tree t;
    parse_toml_in_arena(filename, toml, &t, t.root_id());
    return t;
}

Tree parse_toml_in_arena(csubstr toml)
{
    Tree t;
    parse_toml_in_arena({}, toml, &t, t.root_id());
    return t;
}

//-----------------------------------------------------------------------------
// parse_toml_file overloads

void parse_toml_file(csubstr filename, Tree *t, id_type node_id)
{
    parse_toml_file_impl(filename, t, node_id);
}

void parse_toml_file(csubstr filename, Tree *t)
{
    parse_toml_file_impl(filename, t, t->root_id());
}

void parse_toml_file(csubstr filename, NodeRef node)
{
    parse_toml_file_impl(filename, node.tree(), node.id());
}

Tree parse_toml_file(csubstr filename)
{
    Tree t;
    parse_toml_file_impl(filename, &t, t.root_id());
    return t;
}

} // namespace yml
} // namespace c4
