/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM, TK_HEXNUM, TK_NEGNUM

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +",                TK_NOTYPE},   // spaces
  // The first char '\' means C Escape character "\\+" ==> "\+". '\+' means Regex escape character
  // Finally only plain +.
  {"\\+",               '+'},       // plus
  {"==",                TK_EQ},       // equal
  {"^-[0-9]+",          TK_NEGNUM},   // negative number
  {"-",                 '-'},       // minus
  {"\\*",               '*'},       // multi
  {"/",                 '/'},       // div
  {"\\(",               '('},         // left parentheses
  {"\\)",               ')'},         // right parentheses
  {"^0x[0-9a-fA-F]+",   TK_HEXNUM},   // hex number
  {"[0-9]+",            TK_NUM},      // 0-9 one or more number
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  bool neg_2_minus = false;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            break;
          case TK_HEXNUM:
            tokens[nr_token].type = TK_HEXNUM;
            break;
          case '+':
            tokens[nr_token].type = '+';
            break;
          case '-':
            tokens[nr_token].type = '-';
            break;
          case '*':
            tokens[nr_token].type = '*';
            break;
          case '/':
            tokens[nr_token].type = '/';
            break;
          case '(':
            tokens[nr_token].type = '(';
            break;
          case ')':
            tokens[nr_token].type = ')';
            break;
          case TK_NEGNUM:
            if (nr_token == 0 ||
                (tokens[nr_token - 1].type == '+' || tokens[nr_token - 1].type == '-' ||
                 tokens[nr_token - 1].type == '*' || tokens[nr_token - 1].type == '/' ||
                 tokens[nr_token - 1].type == '(' || tokens[nr_token - 1].type == ')' ))
                tokens[nr_token].type = TK_NEGNUM;
            else {
                tokens[nr_token].type = '-';
                neg_2_minus = true;
            }
            break;
          case TK_EQ:
            tokens[nr_token].type = TK_EQ;
            break;
          case TK_NOTYPE:
            break;
          default: TODO();
        }
        if (rules[i].token_type != TK_NOTYPE) {
            memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
            if (neg_2_minus) substr_start++;
            memcpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
        }
        neg_2_minus = false;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static uint32_t get_main_op_position(uint32_t p, uint32_t q)
{
    uint32_t index = 0, main_position = 0;
    int32_t add_sub_pos = -1;
    int32_t mul_div_pos = -1;
    bool filter = false;

    for (index = p; index <= q; index++) {
        if (tokens[index].type == '(') {
            filter = true;
        } else if (tokens[index].type == ')') {
            filter = false;
        }
        if (true == filter) continue;
        if (tokens[index].type == '+' ||tokens[index].type == '-') {
            add_sub_pos = index;
        }
        if ((tokens[index].type == '*' ||tokens[index].type == '/') && 
            add_sub_pos == -1) {
            mul_div_pos = index;
        }
    }

    if (-1 != add_sub_pos) {
        main_position = add_sub_pos;
    } else if (-1 != mul_div_pos) {
        main_position = mul_div_pos;
    }

    return main_position;
}


bool check_parentheses(uint32_t p, uint32_t q)
{
    uint32_t parentheses_depth = 0, index = 0;
    if ('(' != tokens[p].type || ')' != tokens[q].type) {
        if ('(' == tokens[p].type || ')' == tokens[q].type) {
            Warning("parentheses do not match");
        }
        return false;
    }

    for (index = p; index <= q; index++) {
        if ('(' == tokens[index].type) {
            parentheses_depth++;
        }
        if (')' == tokens[index].type) {
            if (0 == parentheses_depth) {
                Warning("express invalid, more ')'");
                return false;
            }
            parentheses_depth--;
        }
        if (parentheses_depth == 0 && index != q) {
            Warning("express invalid");
            return false;
        }
    }
    Log("check parentheses success");
    return true;
}


int32_t eval(uint32_t p, uint32_t q){
  uint32_t val1 = 0, val2 = 0;
  uint32_t op = 0;
  if (p > q) {
    /* Bad expression */
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
     switch (tokens[p].type) {
        case TK_NUM:        return atoi(tokens[p].str);
        case TK_HEXNUM:     return strtol(tokens[p].str, NULL, 0);
        case TK_NEGNUM:     return atoi(tokens[p].str);
     }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    op = get_main_op_position(p, q);
    if (op == 0) {
        Warning("The express is invalid, please check");
        return 0;
    }
    val1 = eval(p, op - 1);
    val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': 
        if (0 == val2) {
            panic("Division by zero is illegal\n");
        }
        return val1 / val2;
      default: assert(0);
    }
  }
  return 0;
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  *success = true;
  return eval(0,nr_token - 1);
}
