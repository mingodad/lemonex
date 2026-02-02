# LEXGEN Quick Start Guide

## What is LEXGEN?

LEXGEN is a standalone lexer generator library extracted from Lemonex. It allows you to generate lexical analyzers (tokenizers) from regular expressions.

## Quick Examples

### 1. C Example (5 minutes)

```bash
# Clone or download the repository
cd lemonex

# Compile the example
make -f Makefile.lexgen example

# Run it
./lexgen_example
```

**Output:**
```
=== LEXGEN Library Example ===

1. Lexer context initialized
   ✓ Pattern compiled successfully
   ...
   ✓ UTF-8 support demonstrated
   ✓ Code generated to: lexer_generated.c
```

### 2. Lua Extension (5 minutes)

```bash
# Install Lua (if needed)
sudo apt-get install lua5.3 liblua5.3-dev

# Build the Lua extension
make -f Makefile.lexgen lua

# Run the demo
lua5.3 lexgen_demo.lua
```

**Output:**
```
=== LEXGEN Lua Extension Demo ===

Library version: LEXGEN 1.0
✓ 12 token patterns compiled
✓ Code generated to: demo_lexer.c
✓ UTF-8 support demonstrated
```

## Your First Lexer in C

```c
#include "lexgen.h"

int main() {
    // Initialize
    struct LexerContext ctx;
    lexgen_init(&ctx, lexgen_get_default_classlist());
    
    // Define tokens
    struct symbol *sym_id = create_symbol("ID", 1);
    lexgen_parse_regex(&ctx, "[a-zA-Z_]\\w*", 
                       "INITMODE", sym_id, NULL, NULL, 1);
    
    struct symbol *sym_num = create_symbol("NUM", 2);
    lexgen_parse_regex(&ctx, "[0-9]+", 
                       "INITMODE", sym_num, NULL, NULL, 1);
    
    // Generate code
    FILE *out = fopen("my_lexer.c", "w");
    int lineno = 1;
    lexgen_generate_code(&ctx, out, &lineno);
    fclose(out);
    
    printf("Lexer generated! States: %d\n", ctx.next_lxstate_id - 1);
    
    // Cleanup
    lexgen_cleanup(&ctx);
    return 0;
}
```

**Compile and run:**
```bash
gcc -o my_lexer_gen my_program.c lexgen.c
./my_lexer_gen
```

## Your First Lexer in Lua

```lua
local lexgen = require("lexgen_lua")

-- Create context
local ctx = lexgen.new()

-- Define tokens
ctx:parse_regex("[a-zA-Z_]\\w*", "IDENTIFIER")
ctx:parse_regex("[0-9]+", "NUMBER")
ctx:parse_regex("[+\\-*/]", "OPERATOR")
ctx:parse_regex("\\s+", "WHITESPACE", "INITMODE", nil, nil, false)

-- Generate code
ctx:generate_code("my_lexer.c")

print("Done! States created:", ctx:get_state_count())
```

**Run:**
```bash
lua5.3 my_lexer.lua
```

## Supported Regular Expression Syntax

| Pattern | Description | Example |
|---------|-------------|---------|
| `abc` | Literal text | `"hello"` |
| `[abc]` | Character class | `[0-9]` for digits |
| `[a-z]` | Character range | `[a-zA-Z]` for letters |
| `[^abc]` | Negated class | `[^0-9]` non-digits |
| `.` | Any character | `"a.b"` matches a_b, aXb, etc. |
| `*` | Zero or more | `"a*"` matches "", "a", "aa", etc. |
| `+` | One or more | `"a+"` matches "a", "aa", etc. |
| `?` | Optional | `"a?"` matches "" or "a" |
| `\|` | Alternation | `"(cat\|dog)"` |
| `()` | Grouping | `"(ab)+"` |
| `\\l` | Letters | Unicode letter class |
| `\\d` | Digits | Unicode digit class |
| `\\w` | Word chars | Letters + digits + _ |
| `\\s` | Whitespace | Space, tab, newline, etc. |

## Example Patterns

```c
// Programming language tokens
"[a-zA-Z_]\\w*"         // Identifiers
"[0-9]+"                // Integers
"[0-9]+\\.[0-9]+"       // Floats
"(if|else|while|for)"   // Keywords
"[+\\-*/=<>]+"          // Operators
"\"[^\"]*\""            // Strings
"//[^\\n]*"             // Line comments
"/\\*([^*]|\\*[^/])*\\*/" // Block comments (simplified)
"\\s+"                  // Whitespace
```

## Multi-Mode Lexer Example

```lua
local ctx = lexgen.new()

-- Main mode
ctx:parse_regex("[a-z]+", "WORD", "INITMODE")
ctx:parse_regex("/\\*", "START_COMMENT", "INITMODE", "COMMENT")

-- Comment mode
ctx:parse_regex("\\*/", "END_COMMENT", "COMMENT", "INITMODE")
ctx:parse_regex(".", "COMMENT_TEXT", "COMMENT")

ctx:generate_code("multimode_lexer.c")
```

## Build Options

```bash
# Static library
make -f Makefile.lexgen lib
# Creates: liblexgen.a

# Shared library
make -f Makefile.lexgen shared
# Creates: liblexgen.so

# C example
make -f Makefile.lexgen example
# Creates: lexgen_example

# Lua extension
make -f Makefile.lexgen lua
# Creates: lexgen_lua.so

# Run tests
make -f Makefile.lexgen test

# Install system-wide
sudo make -f Makefile.lexgen install
```

## Files to Read

1. **README_LEXGEN.md** - Complete documentation
2. **lexgen.h** - API reference in header comments
3. **lexgen_example.c** - C usage examples
4. **lexgen_demo.lua** - Lua usage examples
5. **IMPLEMENTATION_SUMMARY.md** - Implementation details

## Common Use Cases

### 1. Configuration File Parser
```c
lexgen_parse_regex(&ctx, "[a-zA-Z][a-zA-Z0-9_]*", "INITMODE", sym_key, ...);
lexgen_parse_regex(&ctx, "\"[^\"]*\"", "INITMODE", sym_str, ...);
lexgen_parse_regex(&ctx, "[0-9]+", "INITMODE", sym_num, ...);
lexgen_parse_regex(&ctx, "#[^\\n]*", "INITMODE", sym_comment, ...);
```

### 2. Programming Language
```c
lexgen_parse_regex(&ctx, "[a-zA-Z_]\\w*", "INITMODE", sym_id, ...);
lexgen_parse_regex(&ctx, "[0-9]+", "INITMODE", sym_int, ...);
lexgen_parse_regex(&ctx, "(if|else|while)", "INITMODE", sym_kw, ...);
lexgen_parse_regex(&ctx, "\\s+", "INITMODE", sym_ws, ...);
```

### 3. Log File Analyzer
```c
lexgen_parse_regex(&ctx, "[0-9]{4}-[0-9]{2}-[0-9]{2}", "INITMODE", sym_date, ...);
lexgen_parse_regex(&ctx, "[0-9]{2}:[0-9]{2}:[0-9]{2}", "INITMODE", sym_time, ...);
lexgen_parse_regex(&ctx, "(ERROR|WARN|INFO)", "INITMODE", sym_level, ...);
```

## UTF-8 Support

```c
// Read UTF-8 character
int offset;
int codepoint = lxUtf8_nextchar("你好", &offset);

// Write UTF-8 character
char buf[8];
int len = lxUtf8_str(0x4E2D, 0, buf);  // Chinese character
buf[len] = '\0';
```

From Lua:
```lua
-- Decode UTF-8 string
local codepoints = lexgen.utf8_decode("Hello 世界")

-- Encode codepoint
local char = lexgen.utf8_char(0x4E2D)  -- 中
```

## Performance Tips

1. **Use character classes instead of alternation**
   - Good: `[abc]`
   - Slower: `(a|b|c)`

2. **Use `\\w` instead of explicit ranges**
   - Good: `\\w+`
   - Slower: `[a-zA-Z0-9_]+`

3. **Simplify patterns**
   - Good: `[0-9]+`
   - Slower: `[0-9][0-9]*`

4. **Be specific with terminators**
   - Good: `[^\\n]*` (stops at newline)
   - Slower: `.*` (stops at any delimiter)

## Troubleshooting

**Pattern not compiling:**
- Check escape sequences (use `\\` for backslash)
- Ensure balanced parentheses
- Verify character class syntax

**Lua extension not loading:**
```bash
# Set LUA_CPATH
export LUA_CPATH='./?.so;'$LUA_CPATH

# Or install system-wide
sudo make -f Makefile.lexgen install
```

**Build errors:**
```bash
# Install development tools
sudo apt-get install build-essential

# For Lua
sudo apt-get install lua5.3 liblua5.3-dev
```

## Next Steps

1. Try the examples: `make -f Makefile.lexgen test`
2. Read the full documentation: `README_LEXGEN.md`
3. Create your own lexer patterns
4. Integrate with your parser
5. Explore advanced features (multi-mode, UTF-8, etc.)

## Getting Help

- Read `README_LEXGEN.md` for complete API documentation
- Check `IMPLEMENTATION_SUMMARY.md` for technical details
- Look at `lexgen_example.c` and `lexgen_demo.lua` for examples
- File issues on the repository

## License

Public Domain - No copyright claimed.

---

**Ready to generate lexers!** 🚀
