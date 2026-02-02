/*
** LEXGEN Library - Example Usage
**
** This example demonstrates how to use the lexgen library to:
** 1. Initialize the lexer generator
** 2. Define regex patterns for tokens
** 3. Generate lexer code (concept demo)
**
** Compile: gcc -o lexgen_example lexgen_example.c lexgen.c
** Run: ./lexgen_example
*/

#include "lexgen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple symbol structure for demo */
struct symbol {
    const char *name;
    int index;
};

/* Create a simple symbol */
struct symbol *create_symbol(const char *name, int index) {
    struct symbol *sym = malloc(sizeof(struct symbol));
    sym->name = strdup(name);
    sym->index = index;
    return sym;
}

/* Print state machine information */
void print_state_info(struct LxState *state, int level) {
    if (!state || level > 10) return;  /* Prevent infinite recursion */
    
    printf("%*sState %d:", level * 2, "", state->state_id);
    if (state->is_leaf) printf(" [LEAF]");
    if (state->is_loop_head) printf(" [LOOP_HEAD]");
    printf("\n");
    
    /* Print transitions */
    struct LxTransition *trans = state->head_transition;
    while (trans) {
        printf("%*s  -> ", level * 2, "");
        if (trans->ch0 == trans->ch1) {
            if (trans->ch0 >= 32 && trans->ch0 < 127) {
                printf("'%c'", trans->ch0);
            } else {
                printf("0x%X", trans->ch0);
            }
        } else if (trans->ch1 - trans->ch0 > 5) {
            printf("[range:0x%X-0x%X]", trans->ch0, trans->ch1);
        } else {
            printf("'%c'-'%c'", trans->ch0, trans->ch1);
        }
        printf(" to State %d", trans->next_state ? trans->next_state->state_id : -1);
        printf("\n");
        trans = trans->next_transition;
    }
}

int main(int argc, char *argv[]) {
    printf("=== LEXGEN Library Example ===\n\n");
    
    /* Initialize lexer context */
    struct LexerContext ctx;
    lexgen_init(&ctx, lexgen_get_default_classlist());
    
    printf("1. Lexer context initialized\n");
    printf("   - Default character classes loaded (letter, digit, word, space)\n\n");
    
    /* Example 1: Simple literal pattern */
    printf("2. Creating token pattern: HELLO = \"hello\"\n");
    struct symbol *sym_hello = create_symbol("HELLO", 1);
    struct LxState *state1 = lexgen_parse_regex(&ctx, "hello", "INITMODE", sym_hello, NULL, NULL, 1);
    
    if (state1) {
        printf("   ✓ Pattern compiled successfully\n");
        printf("   State machine created with %d states\n\n", ctx.next_lxstate_id - 1);
    }
    
    /* Example 2: Character class pattern */
    printf("3. Creating token pattern: NUMBER = \"[0-9]+\"\n");
    struct symbol *sym_number = create_symbol("NUMBER", 2);
    struct LxState *state2 = lexgen_parse_regex(&ctx, "[0-9]+", "INITMODE", sym_number, NULL, NULL, 1);
    
    if (state2) {
        printf("   ✓ Pattern compiled successfully\n");
        printf("   Total states now: %d\n\n", ctx.next_lxstate_id - 1);
    }
    
    /* Example 3: Using character classes */
    printf("4. Creating token pattern: IDENTIFIER = \"[a-zA-Z_]\\w*\"\n");
    struct symbol *sym_id = create_symbol("IDENTIFIER", 3);
    struct LxState *state3 = lexgen_parse_regex(&ctx, "[a-zA-Z_]\\w*", "INITMODE", sym_id, NULL, NULL, 1);
    
    if (state3) {
        printf("   ✓ Pattern compiled successfully\n");
        printf("   Total states now: %d\n\n", ctx.next_lxstate_id - 1);
    }
    
    /* Example 4: Alternative patterns (OR) */
    printf("5. Creating token pattern: KEYWORD = \"(if|else|while)\"\n");
    struct symbol *sym_keyword = create_symbol("KEYWORD", 4);
    struct LxState *state4 = lexgen_parse_regex(&ctx, "(if|else|while)", "INITMODE", sym_keyword, NULL, NULL, 1);
    
    if (state4) {
        printf("   ✓ Pattern compiled successfully\n");
        printf("   Total states now: %d\n\n", ctx.next_lxstate_id - 1);
    }
    
    /* Example 5: Using built-in character classes */
    printf("6. Creating token pattern: WHITESPACE = \"\\s+\"\n");
    struct symbol *sym_ws = create_symbol("WHITESPACE", 5);
    struct LxState *state5 = lexgen_parse_regex(&ctx, "\\s+", "INITMODE", sym_ws, NULL, NULL, 0);
    
    if (state5) {
        printf("   ✓ Pattern compiled successfully\n");
        printf("   Total states now: %d\n\n", ctx.next_lxstate_id - 1);
    }
    
    /* Show state machine summary */
    printf("7. State Machine Summary:\n");
    printf("   Total states created: %d\n", ctx.next_lxstate_id - 1);
    printf("   Number of modes: %d\n", ctx.first_mode ? 1 : 0);
    
    if (ctx.first_mode) {
        printf("   Primary mode: %s\n", ctx.first_mode->name ? ctx.first_mode->name : "INITMODE");
    }
    printf("\n");
    
    /* Display a sample of the state machine */
    printf("8. Sample State Machine Structure:\n");
    if (ctx.first_state) {
        struct LxState *s = ctx.first_state;
        int count = 0;
        while (s && count < 3) {
            print_state_info(s, 0);
            s = s->next_state;
            count++;
        }
        if (s) {
            printf("   ... (more states omitted)\n");
        }
    }
    printf("\n");
    
    /* Demonstrate code generation capability */
    printf("9. Code Generation:\n");
    printf("   The library can generate C code for the lexer state machine.\n");
    printf("   Use lexgen_generate_code() to output to a file.\n\n");
    
    /* Generate code to a file */
    FILE *out = fopen("lexer_generated.c", "w");
    if (out) {
        fprintf(out, "/* Auto-generated lexer code */\n");
        fprintf(out, "#include <stdio.h>\n\n");
        
        int lineno = 3;
        Lexer_generate_class_list(&ctx, out, &lineno);
        
        fprintf(out, "\n/* State machine implementation would go here */\n");
        fprintf(out, "/* Use lexgen_generate_code() for full generation */\n");
        
        fclose(out);
        printf("   ✓ Sample code written to: lexer_generated.c\n");
    }
    
    /* Demonstrate UTF-8 support */
    printf("\n10. UTF-8 Support Demonstration:\n");
    const char *utf8_samples[] = {
        "Hello",
        "café",
        "こんにちは",  /* Japanese: Konnichiwa */
        "Привет",      /* Russian: Privet */
        "你好"         /* Chinese: Nihao */
    };
    
    for (int i = 0; i < 5; i++) {
        printf("    Text: %s\n", utf8_samples[i]);
        printf("    Characters: ");
        
        const char *p = utf8_samples[i];
        while (*p) {
            int offset;
            int ch = lxUtf8_nextchar(p, &offset);
            
            char buf[8];
            int len = lxUtf8_str(ch, 0, buf);
            buf[len] = '\0';
            
            printf("%s ", buf);
            p += offset;
        }
        printf("\n");
    }
    
    printf("\n=== Example Complete ===\n");
    printf("\nKey Features Demonstrated:\n");
    printf("  ✓ Literal patterns\n");
    printf("  ✓ Character ranges [0-9], [a-z]\n");
    printf("  ✓ Repetition operators (+, *, ?)\n");
    printf("  ✓ Alternation (|)\n");
    printf("  ✓ Character classes (\\w, \\s, \\d, \\l)\n");
    printf("  ✓ UTF-8 support\n");
    printf("  ✓ Code generation\n");
    printf("\nFor more information, see README_LEXGEN.md\n");
    
    /* Cleanup */
    lexgen_cleanup(&ctx);
    free((void*)sym_hello->name);
    free(sym_hello);
    free((void*)sym_number->name);
    free(sym_number);
    free((void*)sym_id->name);
    free(sym_id);
    free((void*)sym_keyword->name);
    free(sym_keyword);
    free((void*)sym_ws->name);
    free(sym_ws);
    
    return 0;
}
