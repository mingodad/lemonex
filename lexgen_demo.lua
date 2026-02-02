#!/usr/bin/env lua
--[[
** LEXGEN Lua Extension Demo
**
** This script demonstrates how to use the lexgen library from Lua
** to create lexer definitions at runtime.
**
** Usage: lua lexgen_demo.lua
**
** Requirements:
**   - lexgen_lua.so (or .dll on Windows) built and in LUA_CPATH
**   - Lua 5.1 or later
]]

-- Try to load the module
local ok, lexgen = pcall(require, "lexgen_lua")
if not ok then
    print("Error: Cannot load lexgen_lua module")
    print("Make sure lexgen_lua.so is built and in your LUA_CPATH")
    print("\nBuild with:")
    print("  gcc -shared -fPIC -o lexgen_lua.so lexgen_lua.c lexgen.c -I/usr/include/lua5.3 -llua5.3")
    print("\nOr add to LUA_CPATH:")
    print("  export LUA_CPATH='./?.so;'..package.cpath")
    os.exit(1)
end

print("=== LEXGEN Lua Extension Demo ===\n")

-- Print version
print("Library version:", lexgen.version())
print()

-- Create a new lexer context
print("1. Creating lexer context...")
local ctx = lexgen.new()
print("   ✓ Context created:", ctx)
print()

-- Define some token patterns
print("2. Defining token patterns:")
print()

-- Numbers
print("   a) INTEGER = \"[0-9]+\"")
local state1, err1 = ctx:parse_regex("[0-9]+", "INTEGER", "INITMODE")
if state1 then
    print("      ✓ Pattern compiled")
else
    print("      ✗ Error:", err1)
end

-- Floating point numbers
print("   b) FLOAT = \"[0-9]+\\.[0-9]+\"")
local state2, err2 = ctx:parse_regex("[0-9]+\\.[0-9]+", "FLOAT", "INITMODE")
if state2 then
    print("      ✓ Pattern compiled")
else
    print("      ✗ Error:", err2)
end

-- Identifiers
print("   c) IDENTIFIER = \"[a-zA-Z_]\\w*\"")
local state3, err3 = ctx:parse_regex("[a-zA-Z_]\\w*", "IDENTIFIER", "INITMODE")
if state3 then
    print("      ✓ Pattern compiled")
else
    print("      ✗ Error:", err3)
end

-- Keywords
print("   d) IF = \"if\"")
ctx:parse_regex("if", "IF", "INITMODE")
print("      ✓ Pattern compiled")

print("   e) ELSE = \"else\"")
ctx:parse_regex("else", "ELSE", "INITMODE")
print("      ✓ Pattern compiled")

print("   f) WHILE = \"while\"")
ctx:parse_regex("while", "WHILE", "INITMODE")
print("      ✓ Pattern compiled")

print("   g) FOR = \"for\"")
ctx:parse_regex("for", "FOR", "INITMODE")
print("      ✓ Pattern compiled")

-- Operators
print("   h) PLUS = \"+\"")
ctx:parse_regex("+", "PLUS", "INITMODE")
print("      ✓ Pattern compiled")

print("   i) MINUS = \"-\"")
ctx:parse_regex("-", "MINUS", "INITMODE")
print("      ✓ Pattern compiled")

print("   j) EQUALS = \"=\"")
ctx:parse_regex("=", "EQUALS", "INITMODE")
print("      ✓ Pattern compiled")

-- Whitespace (not captured)
print("   k) WHITESPACE = \"\\s+\" (not captured)")
ctx:parse_regex("\\s+", "WHITESPACE", "INITMODE", nil, nil, false)
print("      ✓ Pattern compiled")

-- Comments
print("   l) COMMENT = \"//[^\\n]*\"")
ctx:parse_regex("//[^\n]*", "COMMENT", "INITMODE", nil, nil, false)
print("      ✓ Pattern compiled")

print()

-- Show statistics
print("3. Lexer statistics:")
local state_count = ctx:get_state_count()
local mode_name = ctx:get_mode_name()
print("   Total states created:", state_count)
print("   Primary mode:", mode_name)
print()

-- Generate code
print("4. Generating lexer code...")
local output_file = "demo_lexer.c"
local ok, err = ctx:generate_code(output_file)
if ok then
    print("   ✓ Code generated successfully")
    print("   Output file:", output_file)
else
    print("   ✗ Error:", err)
end
print()

-- Demonstrate UTF-8 functions
print("5. UTF-8 Support:")
print()

-- Convert codepoint to UTF-8
print("   a) Unicode to UTF-8:")
local samples = {
    {0x0041, "LATIN CAPITAL LETTER A"},
    {0x00E9, "LATIN SMALL LETTER E WITH ACUTE (é)"},
    {0x4E2D, "CJK UNIFIED IDEOGRAPH (中)"},
    {0x1F600, "GRINNING FACE EMOJI (😀)"}
}

for _, sample in ipairs(samples) do
    local codepoint, desc = sample[1], sample[2]
    local char = lexgen.utf8_char(codepoint)
    print(string.format("      U+%04X -> %s [%s]", codepoint, char, desc))
end
print()

-- Decode UTF-8 string
print("   b) UTF-8 string to codepoints:")
local test_strings = {
    "Hello",
    "café",
    "你好",  -- Chinese
    "Привет",  -- Russian
}

for _, str in ipairs(test_strings) do
    local codepoints = lexgen.utf8_decode(str)
    local codes = {}
    for i, cp in ipairs(codepoints) do
        table.insert(codes, string.format("U+%04X", cp))
    end
    print(string.format("      \"%s\" -> [%s]", str, table.concat(codes, ", ")))
end
print()

-- Show character class constants
print("6. Character Class Constants:")
print("   LETTER:", lexgen.CLASS_LETTER, "(\\l)")
print("   DIGIT:", lexgen.CLASS_DIGIT, "(\\d)")
print("   WORD:", lexgen.CLASS_WORD, "(\\w)")
print("   SPACE:", lexgen.CLASS_SPACE, "(\\s)")
print("   DOT:", lexgen.CLASS_DOT, "(.)")
print()

-- Advanced usage example
print("7. Advanced Usage - Creating a Multi-Mode Lexer:")
print()

local ctx2 = lexgen.new()

-- Main mode
print("   a) Main mode tokens:")
ctx2:parse_regex("[a-zA-Z_]\\w*", "ID", "INITMODE")
ctx2:parse_regex("[0-9]+", "NUM", "INITMODE")
ctx2:parse_regex("/\\*", "START_COMMENT", "INITMODE", "COMMENT")  -- Switch to COMMENT mode
print("      ✓ Tokens defined with mode switching")

-- Comment mode
print("   b) Comment mode tokens:")
ctx2:parse_regex("\\*/", "END_COMMENT", "COMMENT", "INITMODE")  -- Switch back to INITMODE
ctx2:parse_regex(".", "COMMENT_CHAR", "COMMENT")
print("      ✓ Comment mode tokens defined")

print()
print("   Total states in multi-mode lexer:", ctx2:get_state_count())
print()

-- Configuration example
print("8. Configuration Options:")
ctx2:set_debug_level(1)
print("   ✓ Debug level set to 1")
ctx2:set_nesting_depth(64)
print("   ✓ Nesting depth set to 64")
print()

print("=== Demo Complete ===\n")
print("Key Features Demonstrated:")
print("  ✓ Creating lexer context from Lua")
print("  ✓ Defining token patterns with regex")
print("  ✓ Multiple token types (keywords, identifiers, numbers, operators)")
print("  ✓ Character classes (\\w, \\s, \\d)")
print("  ✓ Non-capturing tokens (whitespace, comments)")
print("  ✓ Generating C code for lexer")
print("  ✓ UTF-8 encoding/decoding")
print("  ✓ Multi-mode lexers with mode switching")
print("  ✓ Configuration options")
print()
print("Generated files:")
print("  - " .. output_file .. " (lexer implementation)")
print()
print("Next steps:")
print("  1. Examine the generated C code")
print("  2. Integrate with your parser")
print("  3. Create custom token patterns")
print("  4. Use in your Lua-based build system")
print()
print("For more information, see README_LEXGEN.md")
