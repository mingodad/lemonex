# LEXGEN - Lexer Generator Library

LEXGEN is a self-contained lexer generator library extracted from the Lemonex parser generator. It can be used independently to generate lexical analyzers from regular expressions.

## Features

- **UTF-8 Support**: Full Unicode support with built-in character classes
- **Rich Regular Expressions**: Support for *, +, ?, |, [], character classes, and grouping
- **Character Classes**: Built-in classes for letters (\l), digits (\d), words (\w), and spaces (\s)
- **Multi-Mode Lexers**: Support for multiple lexer modes with nesting
- **Code Generation**: Generate efficient C code for state machine implementation
- **Standalone**: No external dependencies except standard C library
- **Language Bindings**: Lua extension for runtime lexer generation

## Quick Start

### C API

```c
#include "lexgen.h"

int main() {
    // Initialize context
    struct LexerContext ctx;
    lexgen_init(&ctx, lexgen_get_default_classlist());
    
    // Parse a regex pattern
    struct symbol *sym = create_symbol("NUMBER", 1);
    struct LxState *state = lexgen_parse_regex(&ctx, "[0-9]+", 
                                                "INITMODE", sym, 
                                                NULL, NULL, 1);
    
    // Generate code
    FILE *out = fopen("lexer.c", "w");
    int lineno = 1;
    lexgen_generate_code(&ctx, out, &lineno);
    fclose(out);
    
    // Cleanup
    lexgen_cleanup(&ctx);
    return 0;
}
```

### Lua API

```lua
local lexgen = require("lexgen_lua")

-- Create context
local ctx = lexgen.new()

-- Define tokens
ctx:parse_regex("[0-9]+", "NUMBER")
ctx:parse_regex("[a-zA-Z_]\\w*", "IDENTIFIER")
ctx:parse_regex("\\s+", "WHITESPACE", "INITMODE", nil, nil, false)

-- Generate code
ctx:generate_code("lexer.c")

print("States created:", ctx:get_state_count())
```

## Building

### Library

```bash
# Compile the library
gcc -c -o lexgen.o lexgen.c

# Create static library
ar rcs liblexgen.a lexgen.o

# Or create shared library
gcc -shared -fPIC -o liblexgen.so lexgen.c
```

### Example Program

```bash
gcc -o lexgen_example lexgen_example.c lexgen.c
./lexgen_example
```

### Lua Extension

```bash
# Lua 5.3
gcc -shared -fPIC -o lexgen_lua.so lexgen_lua.c lexgen.c \
    -I/usr/include/lua5.3 -llua5.3

# Lua 5.4
gcc -shared -fPIC -o lexgen_lua.so lexgen_lua.c lexgen.c \
    -I/usr/include/lua5.4 -llua5.4

# Run demo
lua lexgen_demo.lua
```

## API Reference

### Core Types

#### LexerContext

Main context structure holding lexer generation state.

```c
struct LexerContext {
    int next_lxstate_id;        // Next available state ID
    struct LxMode *first_mode;  // First mode in list
    struct LxMode *last_mode;   // Last mode in list
    struct LxState *first_state;// First state in list
    struct LxState *last_state; // Last state in list
    struct LxState *curr_state; // Current state being built
    const int *lxclasslist;     // Character class lookup table
    int nesting_depth;          // Maximum nesting depth
    int debug_level;            // Debug output level (0-2)
};
```

#### LxState

Represents a state in the lexer state machine.

```c
struct LxState {
    int state_id;                    // Unique state identifier
    int is_leaf;                     // Accepting/final state flag
    struct symbol* matchedsym;       // Matched symbol
    const char *code;                // Action code
    struct LxTransition* head_transition; // Transition list
    // ... (see lexgen.h for full definition)
};
```

#### LxTransition

Represents a transition between states.

```c
struct LxTransition {
    int ch0;                    // Start of character range
    int ch1;                    // End of character range
    int tx_type;                // Transition type
    struct LxState *next_state; // Target state
    // ... (see lexgen.h for full definition)
};
```

### Main Functions

#### lexgen_init

Initialize lexer context.

```c
void lexgen_init(struct LexerContext *ctx, const int *classlist);
```

- **ctx**: Context to initialize
- **classlist**: Character class lookup table (use `lexgen_get_default_classlist()`)

#### lexgen_parse_regex

Parse regex and create lexer states.

```c
struct LxState *lexgen_parse_regex(
    struct LexerContext *ctx,
    const char *regex,
    const char *mode_name,
    struct symbol *sym,
    const char *next_mode,
    const char *action_code,
    int is_capturing
);
```

- **ctx**: Lexer context
- **regex**: Regular expression pattern
- **mode_name**: Lexer mode name (e.g., "INITMODE")
- **sym**: Symbol associated with this pattern
- **next_mode**: Mode to switch to after match (NULL for no switch)
- **action_code**: Code to execute on match (NULL for none)
- **is_capturing**: 1 to capture matched text, 0 otherwise
- **Returns**: Final state or NULL on error

#### lexgen_generate_code

Generate C code for lexer state machine.

```c
void lexgen_generate_code(
    struct LexerContext *ctx,
    FILE *output,
    int *lineno
);
```

- **ctx**: Lexer context
- **output**: Output file stream
- **lineno**: Current line number (updated during generation)

#### Lexer_generate_class_list

Generate character class lookup tables.

```c
void Lexer_generate_class_list(
    struct LexerContext *ctx,
    FILE *fp,
    int *lineno
);
```

#### lexgen_cleanup

Cleanup lexer context.

```c
void lexgen_cleanup(struct LexerContext *ctx);
```

#### lexgen_get_default_classlist

Get default character class list (built-in Unicode tables).

```c
const int *lexgen_get_default_classlist(void);
```

### UTF-8 Functions

#### lxUtf8_nextchar

Read one UTF-8 character from string.

```c
int lxUtf8_nextchar(const char *p, int *offset);
```

- **p**: Pointer to UTF-8 string
- **offset**: Output: number of bytes consumed
- **Returns**: Unicode codepoint

#### lxUtf8_str

Convert Unicode character to UTF-8 string.

```c
int lxUtf8_str(int ch, int display_mode, char *p);
```

- **ch**: Unicode codepoint
- **display_mode**: 1 to show control chars as hex, 0 for normal
- **p**: Output buffer (must be at least 8 bytes)
- **Returns**: Number of bytes written

## Regular Expression Syntax

### Basic Patterns

| Pattern | Description |
|---------|-------------|
| `abc` | Literal string |
| `.` | Any character |
| `[abc]` | Character class (a, b, or c) |
| `[a-z]` | Character range (a through z) |
| `[^abc]` | Negated class (anything except a, b, c) |

### Operators

| Operator | Description |
|----------|-------------|
| `*` | Zero or more (greedy) |
| `+` | One or more (greedy) |
| `?` | Zero or one (optional) |
| `\|` | Alternation (or) |
| `()` | Grouping |

### Character Classes

| Class | Description | Equivalent |
|-------|-------------|------------|
| `\l` | Letters | Unicode letter categories |
| `\d` | Digits | `[0-9]` plus Unicode digits |
| `\w` | Word characters | Letters + digits + underscore |
| `\s` | Whitespace | Space, tab, newline, etc. |

### Escape Sequences

| Sequence | Character |
|----------|-----------|
| `\n` | Newline |
| `\r` | Carriage return |
| `\t` | Tab |
| `\\` | Backslash |
| `\"` | Quote |

### Examples

```
[0-9]+              # One or more digits
[0-9]+\.[0-9]+      # Floating point number
[a-zA-Z_]\w*        # C identifier
"[^"]*"             # Quoted string
(if|else|while)     # Keywords
//[^\n]*            # Line comment
\s+                 # Whitespace
```

## Character Classes

LEXGEN includes comprehensive Unicode character class definitions:

- **Letter class** (\l): All Unicode letter characters (lowercase and uppercase)
- **Digit class** (\d): All Unicode digit characters (0-9 plus other numeral systems)
- **Word class** (\w): Letters + digits + underscore
- **Space class** (\s): All Unicode whitespace characters
- **Dot class** (.): Any character

The character class tables are automatically included in generated code.

## Lexer Modes

LEXGEN supports multiple lexer modes for context-sensitive lexing:

```c
// Define token in INITMODE that switches to COMMENT mode
lexgen_parse_regex(&ctx, "/\\*", "INITMODE", sym1, "COMMENT", NULL, 1);

// Define token in COMMENT mode that switches back
lexgen_parse_regex(&ctx, "\\*/", "COMMENT", sym2, "INITMODE", NULL, 1);

// Other tokens in COMMENT mode
lexgen_parse_regex(&ctx, ".", "COMMENT", sym3, NULL, NULL, 0);
```

### Nesting

Modes can be nested to support recursive structures like nested comments:

```c
ctx.nesting_depth = 64;  // Maximum nesting level

// Recursive entry into same mode
lexgen_parse_regex(&ctx, "/\\*", "COMMENT", sym, "COMMENT", NULL, 1);

// Exit from nested mode  
lexgen_parse_regex(&ctx, "\\*/", "COMMENT", sym, "<", NULL, 1);
```

The special mode name `"<"` means "return to previous mode".

## Lua Extension API

### Module Functions

#### lexgen.new()

Create new lexer context.

```lua
local ctx = lexgen.new()
```

#### lexgen.version()

Get library version string.

```lua
print(lexgen.version())
```

#### lexgen.utf8_char(codepoint)

Convert Unicode codepoint to UTF-8 string.

```lua
local char = lexgen.utf8_char(0x4E2D)  -- Chinese character '中'
```

#### lexgen.utf8_decode(str)

Decode UTF-8 string to array of codepoints.

```lua
local codepoints = lexgen.utf8_decode("Hello")
-- returns {72, 101, 108, 108, 111}
```

### Context Methods

#### ctx:parse_regex(pattern, token_name [, mode [, next_mode [, action_code [, is_capturing]]]])

Parse regex pattern and create states.

```lua
local state = ctx:parse_regex("[0-9]+", "NUMBER")
local state = ctx:parse_regex("\\s+", "WS", "INITMODE", nil, nil, false)
```

#### ctx:generate_code(filename)

Generate lexer code to file.

```lua
local ok, err = ctx:generate_code("lexer.c")
```

#### ctx:get_state_count()

Get number of states created.

```lua
local count = ctx:get_state_count()
```

#### ctx:get_mode_name()

Get name of first/primary mode.

```lua
local mode = ctx:get_mode_name()
```

#### ctx:set_debug_level(level)

Set debug output level (0-2).

```lua
ctx:set_debug_level(1)
```

#### ctx:set_nesting_depth(depth)

Set maximum mode nesting depth.

```lua
ctx:set_nesting_depth(64)
```

### Constants

```lua
lexgen.CLASS_LETTER  -- 1
lexgen.CLASS_DIGIT   -- 2
lexgen.CLASS_WORD    -- 3
lexgen.CLASS_SPACE   -- 4
lexgen.CLASS_DOT     -- 5
```

## Use Cases

### 1. Compiler/Interpreter Development

Generate lexers for programming languages:

```c
lexgen_parse_regex(&ctx, "[a-zA-Z_]\\w*", "INITMODE", sym_id, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "[0-9]+", "INITMODE", sym_num, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "(if|else|while|for)", "INITMODE", sym_kw, NULL, NULL, 1);
```

### 2. Configuration File Parsing

Parse structured text files:

```c
lexgen_parse_regex(&ctx, "[a-zA-Z][a-zA-Z0-9_]*", "INITMODE", sym_key, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "\"[^\"]*\"", "INITMODE", sym_str, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "[0-9]+", "INITMODE", sym_val, NULL, NULL, 1);
```

### 3. Log File Analysis

Extract structured data from logs:

```c
lexgen_parse_regex(&ctx, "[0-9]{4}-[0-9]{2}-[0-9]{2}", "INITMODE", sym_date, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "[0-9]{2}:[0-9]{2}:[0-9]{2}", "INITMODE", sym_time, NULL, NULL, 1);
lexgen_parse_regex(&ctx, "(ERROR|WARN|INFO)", "INITMODE", sym_level, NULL, NULL, 1);
```

### 4. Lua-Based Build Systems

Generate lexers at build time:

```lua
-- build.lua
local lexgen = require("lexgen_lua")

local function generate_lexer(spec, output)
    local ctx = lexgen.new()
    for _, token in ipairs(spec) do
        ctx:parse_regex(token.pattern, token.name)
    end
    ctx:generate_code(output)
end

generate_lexer({
    {pattern = "[a-zA-Z_]\\w*", name = "ID"},
    {pattern = "[0-9]+", name = "NUM"},
}, "lexer.c")
```

### 5. Runtime Code Generation

Generate and compile lexers on-the-fly:

```lua
local function create_custom_lexer(patterns)
    local ctx = lexgen.new()
    for name, pattern in pairs(patterns) do
        ctx:parse_regex(pattern, name)
    end
    
    ctx:generate_code("custom_lexer.c")
    os.execute("gcc -c custom_lexer.c")
    -- Load dynamically...
end
```

## Integration with Lemon Parser Generator

LEXGEN was extracted from Lemonex and works seamlessly with the Lemon parser generator:

1. Generate lexer with LEXGEN
2. Generate parser with Lemon
3. Integrate in your application

```c
// Generated lexer
int lexer_next_token(LexerState *lex, Token *tok);

// Generated parser  
void *ParseAlloc(void *(*mallocProc)(size_t));
void Parse(void *yyp, int yymajor, Token yyminor);

// Integration
LexerState lex;
void *parser = ParseAlloc(malloc);
Token tok;

while (lexer_next_token(&lex, &tok)) {
    Parse(parser, tok.type, tok);
}
Parse(parser, 0, tok);  // End of input
```

## Performance Considerations

- **State Count**: More complex patterns create more states; simplify where possible
- **Character Classes**: Using `\w` is more efficient than `[a-zA-Z0-9_]`
- **Alternation**: `(a|b|c)` creates branches; use `[abc]` when possible
- **Greedy Matching**: `.*` matches as much as possible; use `[^x]*` to stop at delimiter

## Error Handling

Functions return NULL or error indicators on failure:

```c
struct LxState *state = lexgen_parse_regex(&ctx, pattern, ...);
if (!state) {
    fprintf(stderr, "Error: Failed to parse regex\n");
    return -1;
}
```

From Lua:

```lua
local state, err = ctx:parse_regex(pattern, token)
if not state then
    print("Error:", err)
end
```

## Thread Safety

LEXGEN is **not thread-safe**. Each thread should use its own LexerContext:

```c
// Thread 1
struct LexerContext ctx1;
lexgen_init(&ctx1, lexgen_get_default_classlist());

// Thread 2
struct LexerContext ctx2;
lexgen_init(&ctx2, lexgen_get_default_classlist());
```

## Memory Management

- LexerContext manages state/transition allocation internally
- Symbol structures must be managed by caller
- Generated code is written to provided FILE*
- No dynamic cleanup needed for current implementation

## Limitations

1. **Lookahead**: LEXGEN uses lookahead of 1 with fail state
2. **Backtracking**: No backtracking; patterns must be unambiguous
3. **Captures**: Capture definitions cannot overlap in regex
4. **Recursion**: Regex recursion not supported (use mode nesting instead)

## Debugging

Set debug level for diagnostic output:

```c
ctx.debug_level = 1;  // Normal debug output
ctx.debug_level = 2;  // Verbose debug output
```

From Lua:

```lua
ctx:set_debug_level(2)
```

## Contributing

This library is in the public domain. Contributions welcome!

## License

Public Domain - No copyright claimed.
This code is based on the Lemonex parser generator.

## See Also

- Original Lemonex: https://chiselapp.com/user/renjipanicker/repository/lemonex
- Lemon Parser Generator: https://www.sqlite.org/src/doc/trunk/doc/lemon.html

## Contact

For questions or issues with the extracted library, please file an issue on the repository.
