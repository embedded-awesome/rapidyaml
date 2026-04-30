#ifndef _C4_YML_PARSE_TOML_HPP_
#define _C4_YML_PARSE_TOML_HPP_

/** @file parse_toml.hpp Functions to parse TOML and emit as a ryml::Tree.
 *
 * @note This header requires C++17 and the toml++ library.
 * Include <toml++/toml.hpp> before including this header, or define
 * RYML_TOML_INCLUDE_TOMLPLUSPLUS to have this header include it for you.
 */

#ifndef _C4_YML_COMMON_HPP_
#include "c4/yml/common.hpp"
#endif

#ifndef _C4_YML_TREE_HPP_
#include "c4/yml/tree.hpp"
#endif

#ifdef RYML_TOML_INCLUDE_TOMLPLUSPLUS
#include <toml++/toml.hpp>
#endif

#include <string>
#include <string_view>

namespace c4 {
namespace yml {

/** @addtogroup doc_parse
 * @{ */

/** @defgroup doc_parse_toml Parse TOML
 *
 * @brief Functions to parse TOML content and emit it as a ryml::Tree.
 *
 * These functions use the toml++ library (https://github.com/marzer/tomlplusplus)
 * to parse TOML content and convert it to a ryml::Tree structure.
 *
 * @note Requires C++17 and toml++ to be available.
 *
 * @{
 */

//-----------------------------------------------------------------------------

/** @defgroup doc_parse_toml_in_place Parse TOML in place
 *
 * @brief Parse a TOML source buffer and store it in a ryml::Tree.
 *
 * @note Unlike YAML parsing, TOML parsing cannot truly be done "in place"
 * because the toml++ library requires its own internal representation.
 * The parsed content is converted to the ryml::Tree structure.
 *
 * @{
 */

/** Parse TOML content from a string into an existing tree node.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param t The tree to populate
 * @param node_id The node ID to use as root */
RYML_EXPORT void parse_toml_in_place(csubstr filename, csubstr toml, Tree *t, id_type node_id);

/** Parse TOML content from a string into an existing tree node.
 * @param toml The TOML content to parse
 * @param t The tree to populate
 * @param node_id The node ID to use as root */
RYML_EXPORT void parse_toml_in_place(csubstr toml, Tree *t, id_type node_id);

/** Parse TOML content from a string into the root of an existing tree.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param t The tree to populate */
RYML_EXPORT void parse_toml_in_place(csubstr filename, csubstr toml, Tree *t);

/** Parse TOML content from a string into the root of an existing tree.
 * @param toml The TOML content to parse
 * @param t The tree to populate */
RYML_EXPORT void parse_toml_in_place(csubstr toml, Tree *t);

/** Parse TOML content from a string into a NodeRef.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param node The node to populate */
RYML_EXPORT void parse_toml_in_place(csubstr filename, csubstr toml, NodeRef node);

/** Parse TOML content from a string into a NodeRef.
 * @param toml The TOML content to parse
 * @param node The node to populate */
RYML_EXPORT void parse_toml_in_place(csubstr toml, NodeRef node);

/** Parse TOML content from a string and return a new tree.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @return A new tree containing the parsed TOML */
RYML_EXPORT Tree parse_toml_in_place(csubstr filename, csubstr toml);

/** Parse TOML content from a string and return a new tree.
 * @param toml The TOML content to parse
 * @return A new tree containing the parsed TOML */
RYML_EXPORT Tree parse_toml_in_place(csubstr toml);

/** @} */

//-----------------------------------------------------------------------------

/** @defgroup doc_parse_toml_in_arena Parse TOML in arena
 *
 * @brief Parse TOML content by first copying it to the tree's arena.
 *
 * These functions copy the TOML content to the tree's internal arena
 * before parsing, ensuring the content remains valid for the tree's lifetime.
 *
 * @{
 */

/** Parse TOML content from a string into an existing tree node, using the arena.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param t The tree to populate
 * @param node_id The node ID to use as root */
RYML_EXPORT void parse_toml_in_arena(csubstr filename, csubstr toml, Tree *t, id_type node_id);

/** Parse TOML content from a string into an existing tree node, using the arena.
 * @param toml The TOML content to parse
 * @param t The tree to populate
 * @param node_id The node ID to use as root */
RYML_EXPORT void parse_toml_in_arena(csubstr toml, Tree *t, id_type node_id);

/** Parse TOML content from a string into the root of an existing tree, using the arena.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param t The tree to populate */
RYML_EXPORT void parse_toml_in_arena(csubstr filename, csubstr toml, Tree *t);

/** Parse TOML content from a string into the root of an existing tree, using the arena.
 * @param toml The TOML content to parse
 * @param t The tree to populate */
RYML_EXPORT void parse_toml_in_arena(csubstr toml, Tree *t);

/** Parse TOML content from a string into a NodeRef, using the arena.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @param node The node to populate */
RYML_EXPORT void parse_toml_in_arena(csubstr filename, csubstr toml, NodeRef node);

/** Parse TOML content from a string into a NodeRef, using the arena.
 * @param toml The TOML content to parse
 * @param node The node to populate */
RYML_EXPORT void parse_toml_in_arena(csubstr toml, NodeRef node);

/** Parse TOML content from a string and return a new tree, using the arena.
 * @param filename Used in error messages
 * @param toml The TOML content to parse
 * @return A new tree containing the parsed TOML */
RYML_EXPORT Tree parse_toml_in_arena(csubstr filename, csubstr toml);

/** Parse TOML content from a string and return a new tree, using the arena.
 * @param toml The TOML content to parse
 * @return A new tree containing the parsed TOML */
RYML_EXPORT Tree parse_toml_in_arena(csubstr toml);

/** @} */

//-----------------------------------------------------------------------------

/** @defgroup doc_parse_toml_file Parse TOML file
 *
 * @brief Parse TOML content from a file.
 *
 * These functions read and parse TOML content from a file path.
 *
 * @{
 */

/** Parse a TOML file into an existing tree node.
 * @param filename The path to the TOML file
 * @param t The tree to populate
 * @param node_id The node ID to use as root */
RYML_EXPORT void parse_toml_file(csubstr filename, Tree *t, id_type node_id);

/** Parse a TOML file into the root of an existing tree.
 * @param filename The path to the TOML file
 * @param t The tree to populate */
RYML_EXPORT void parse_toml_file(csubstr filename, Tree *t);

/** Parse a TOML file into a NodeRef.
 * @param filename The path to the TOML file
 * @param node The node to populate */
RYML_EXPORT void parse_toml_file(csubstr filename, NodeRef node);

/** Parse a TOML file and return a new tree.
 * @param filename The path to the TOML file
 * @return A new tree containing the parsed TOML */
RYML_EXPORT Tree parse_toml_file(csubstr filename);

/** @} */

/** @} */ // doc_parse_toml

/** @} */ // doc_parse

} // namespace yml
} // namespace c4

#endif /* _C4_YML_PARSE_TOML_HPP_ */
