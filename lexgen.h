/*
** LEXGEN - Lexer Generator Library
** 
** This library provides lexer generation functionality extracted from lemonex.
** It can be used independently to generate lexical analyzers from regular expressions.
**
** The library supports:
** - UTF-8 character encoding
** - Character classes (\l for letters, \d for digits, \w for word, \s for space)
** - Regular expression operators (*, +, ?, |, [])
** - Multiple lexer modes
** - Nested modes with configurable depth
**
** Public Domain - No copyright claimed.
*/

#ifndef LEXGEN_H
#define LEXGEN_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declarations */
struct symbol;

/* Character class constants */
#define LXCLASS_LETTER 1
#define LXCLASS_DIGIT 2
#define LXCLASS_WORD 3
#define LXCLASS_SPACE 4
#define LXCLASS_DOT 5
#define LXCLASS_LAST (LXCLASS_DOT)

/* UTF-8 encoding masks */
#define MASKBITS   0x3F
#define MASK1BYTES 0x80
#define MASK2BYTES 0xC0
#define MASK3BYTES 0xE0
#define MASK4BYTES 0xF0
#define MASK5BYTES 0xF8
#define MASK6BYTES 0xFC

/* Lexer scanner structure for parsing regular expressions */
struct LxScanner {
  const char *p;      /* Current position in input string */
  int ch;             /* Current UTF-8 character */
  int offset;         /* Byte offset of current character */
};

/* Lexer mode structure */
struct LxMode {
  const char *name;              /* Mode name */
  struct LxState *init_state;    /* Pointer to root state in this mode */
  int is_nesting;                /* 1 if this mode is recursive */
  struct LxMode *next_mode;      /* Next mode in list */
};

/* AST node types for regex parsing */
enum LxAstNodeType {
  CHAR_NODE,
  CLASS_NODE,
  NCLASS_NODE,
  STAR_NODE,
  PLUS_NODE,
  OPT_NODE,
  OR_NODE,
  AND_NODE,
};

/* AST node for regex expression tree */
struct LxAstNode {
  int node_type;            /* Type from LxAstNodeType enum */
  struct LxAstNode *lhs;    /* Left child node */
  struct LxAstNode *rhs;    /* Right child node */
  int ch0;                  /* Start character/class */
  int ch1;                  /* End character (for ranges) */
};

/* Transition types */
enum LxTransitionType {
  REGULAR_TX = 0,
  NOT_TX = 1,
  INIT_TX = 2,
  DROP_TX = 3,
  LOOP_TX = 4,
};

/* State transition structure */
struct LxTransition {
  int ch0;                           /* Start of character range */
  int ch1;                           /* End of character range */
  int tx_type;                       /* Transition type */
  int is_drop;                       /* Drop flag */
  int is_capturing;                  /* Capturing flag */
  int is_loop_set;                   /* Loop grouping flag */
  struct LxState *next_state;        /* Target state */
  struct LxTransition* next_transition; /* Next transition in list */
};

/* Lexer state structure */
struct LxState {
  int state_id;                       /* Unique state identifier */
  int is_leaf;                        /* Accepting/final state flag */
  int is_loop_head;                   /* Loop head flag */
  int is_loop_enter;                  /* Loop entry flag */
  int is_loop_return;                 /* Loop return flag */
  int is_processed;                   /* Processing flag */
  struct symbol* matchedsym;          /* Matched symbol (external) */
  struct LxMode *init_mode;           /* Initial mode */
  const char *next_mode;              /* Next mode name */
  const char *code;                   /* Action code */
  struct LxState *star_state;         /* Star state */
  struct LxState *drop_state;         /* Drop state */
  struct LxState *goto_state;         /* Goto state */
  struct LxTransition* not_transition;  /* NOT transitions */
  struct LxTransition* dot_transition;  /* DOT transition */
  struct LxTransition* head_transition; /* Regular transitions */
  struct LxAstNode *root_node;
  struct LxState *next_state;         /* Next state in list */
};

/* Lexer context structure - holds all lexer generation state */
struct LexerContext {
  int next_lxstate_id;          /* Next available state ID */
  struct LxMode *first_mode;    /* First mode in list */
  struct LxMode *last_mode;     /* Last mode in list */
  struct LxState *first_state;  /* First state in list */
  struct LxState *last_state;   /* Last state in list */
  struct LxState *curr_state;   /* Current state being built */
  const int *lxclasslist;       /* Character class lookup table */
  int nesting_depth;            /* Maximum nesting depth */
  int debug_level;              /* Debug output level (0-2) */
};

/*
** UTF-8 utility functions
*/

/* Read one UTF-8 character from string and return character code.
** Updates offset to the number of bytes consumed. */
int lxUtf8_nextchar(const char *p, int *offset);

/* Convert Unicode character to UTF-8 string.
** If display_mode is 1, control characters are shown as hex.
** Returns number of bytes written to p. */
int lxUtf8_str(int ch, int display_mode, char *p);

/*
** Scanner functions for regex parsing
*/

/* Lookahead: peek at next character without consuming */
int lx_lookahead(struct LxScanner *s);

/* Advance scanner to next character */
void lx_advance(struct LxScanner *s);

/*
** Lexer mode management
*/

/* Find existing mode by name */
struct LxMode *LxMode_find(struct LexerContext *ctx, const char *mode_name);

/* Create new lexer mode */
struct LxMode *LxMode_new(struct LexerContext *ctx, const char *mode_name, int is_nesting);

/*
** State and transition management
*/

/* Create a new lexer state */
struct LxState *LxState_create(struct LexerContext *ctx);

/* Create a new transition */
struct LxTransition *LxTransition_create(int ch0, int ch1, int tx_type, struct LxState *next_state);

/* Insert transition into state's transition list */
void LxTransition_insert(struct LxState *state, int ch0, int ch1, int tx_type, struct LxState *next_state);

/* Set drop state for error recovery */
void LxState_set_drop(struct LxState *state, struct LxState *drop_state);

/* Set goto state for optional/alternative paths */
void LxState_set_goto(struct LxState *state, struct LxState *goto_state);

/*
** Regular expression parsing
*/

/* Parse regex string and build state machine.
** Returns the AST root node. */
struct LxAstNode *lx_expr(struct LexerContext *ctx, struct LxScanner *s);

/*
** AST processing
*/

/* Process AST node and generate states/transitions.
** start_state: initial state
** final_state: target final state
** Returns: actual final state after processing */
struct LxState *LxNode_process(
  struct LexerContext *ctx,
  struct LxAstNode *node,
  struct LxState *start_state,
  struct LxState *final_state
);

/*
** State postprocessing
*/

/* Postprocess state machine (normalize, set drops, etc.) */
void LxState_postprocess(struct LexerContext *ctx, struct LxState *state);

/*
** Main API functions
*/

/* Initialize lexer context with character class list */
void lexgen_init(struct LexerContext *ctx, const int *classlist);

/* Parse regex and create lexer states in the given mode.
** Returns final state of the regex. */
struct LxState *lexgen_parse_regex(
  struct LexerContext *ctx,
  const char *regex,
  const char *mode_name,
  struct symbol *sym,
  const char *next_mode,
  const char *action_code,
  int is_capturing
);

/* Generate C code for lexer state machine */
void lexgen_generate_code(
  struct LexerContext *ctx,
  FILE *output,
  int *lineno
);

/* Generate character class lookup tables */
void Lexer_generate_class_list(struct LexerContext *ctx, FILE *fp, int *lineno);

/* Cleanup lexer context (currently no dynamic allocations to free) */
void lexgen_cleanup(struct LexerContext *ctx);

/* Cleanup lexer LxAstNode  */
void lexgen_cleanup_ast(struct LxAstNode *ast);

/* Cleanup lexer LxTransition  */
void lexgen_cleanup_transition(struct LxTransition *ast);

/* Get default character class list (built-in Unicode tables) */
const int *lexgen_get_default_classlist(void);

#ifdef __cplusplus
}
#endif

#endif /* LEXGEN_H */
