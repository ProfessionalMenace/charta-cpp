#include "core.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ch_string ch_str_new(char const *data) {
    ch_string str;
    size_t len = strlen(data);
    str.data = malloc(len);
    strcpy(str.data, data);
    str.len = len;
    str.size = len;
    return str;
}

void ch_str_delete(ch_string *str) {
    free(str->data);
    str->data = NULL;
}

ch_value ch_valof_int(int n) {
    return (ch_value){.kind = CH_VALK_INT, .value.i = n};
}

ch_value ch_valof_float(float n) {
    return (ch_value){.kind = CH_VALK_FLOAT, .value.f = n};
}

// UTF32 codepoint
ch_value ch_valof_char(int n) {
    return (ch_value){.kind = CH_VALK_CHAR, .value.i = n};
}

ch_value ch_valof_string(ch_string n) {
    return (ch_value){.kind = CH_VALK_STRING, .value.s = n};
}

ch_value ch_valof_bool(char n) {
    return (ch_value){.kind = CH_VALK_BOOL, .value.b = n};
}

char *ch_valk_name(ch_value_kind k) {
    switch (k) {
    case CH_VALK_INT:
        return "int";
    case CH_VALK_FLOAT:
        return "float";
    case CH_VALK_BOOL:
        return "bool";
    case CH_VALK_CHAR:
        return "char";
    case CH_VALK_STRING:
        return "string";
    }
}

char ch_valas_bool(ch_value v) {
    if (v.kind != CH_VALK_BOOL) {
        printf("ERR: Expected 'bool', got '%s'\n", ch_valk_name(v.kind));
        exit(1);
    }
    return v.value.b;
}

void ch_val_delete(ch_value *val) {
    if (val->kind == CH_VALK_STRING) {
        ch_str_delete(&val->value.s);
    }
    val->kind = -1;
}

ch_stack_node *ch_stk_new() { return NULL; }

void ch_stk_push(ch_stack_node **stk, ch_value val) {
    ch_stack_node *new = malloc(sizeof(ch_stack_node));
    new->val = val;
    new->next = *stk;
    *stk = new;
}

ch_value ch_stk_pop(ch_stack_node **stk) {
    ch_value v = (*stk)->val;
    *stk = (*stk)->next;
    return v;
}

ch_stack_node *ch_stk_args(ch_stack_node **from, size_t n) {
    ch_stack_node *args = *from;
    ch_stack_node *bot = *from;
    for (size_t i = 0; i < n; ++i) {
        if (bot == NULL) {
          printf("ERR: Tried to pop '%zu' arguments, got '%zu", n, i);
          exit(1);
        }
        bot = bot->next;
    }
    if (bot) {    
      *from = bot->next;
      bot->next = NULL;    
    } else {
      *from = NULL;
    }
    return args;
}

///! MOVES
void ch_stk_append(ch_stack_node **to, ch_stack_node *from) {
  if (from) {
    ch_stack_node *end = from;
    while (end->next != NULL) {
        end = end->next;
    }
    end->next = *to;
    *to = from;
  }
}

void ch_stk_delete(ch_stack_node **stk) {
    if (stk == NULL)
        return;
    ch_val_delete(&(*stk)->val);
    ch_stk_delete(&(*stk)->next);
    free(*stk);
    *stk = NULL;
}

ch_stack_node *print(ch_stack_node **full) {
  ch_value v = ch_stk_pop(full);
  switch (v.kind) {
  case CH_VALK_INT:
    printf("%d\n", v.value.i);
    break;
  case CH_VALK_FLOAT:
    printf("%f\n", v.value.f);
    break;
  case CH_VALK_BOOL:
    printf("%b\n", v.value.b);
    break;
  case CH_VALK_CHAR:
    printf("'\\U%x'\n", v.value.i);
    break;
  case CH_VALK_STRING:
    printf("%s\n", v.value.s.data);
    break;
  }
  return NULL;
}    
