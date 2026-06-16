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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\\n\", result); "
"  return 0; "
"}";

static uint32_t expr_len = 0;

static uint32_t choose(uint32_t max_num) {
    return rand() % max_num;
}

static void gen_space() {
    int n = choose(3);  // 0, 1, or 2 spaces
    for (int i = 0; i < n; i++) {
        buf[expr_len++] = ' ';
    }
}

static void gen_num(){
    int value = rand();
    expr_len += sprintf(buf + expr_len, "%u", value);
}

static void gen(char ch) {
    if (ch == '(') {
        gen_space();
    }
    buf[expr_len++] = ch;
}

static void gen_rand_op(){
    gen_space();    // space before operator
    uint32_t op_index = choose(4);
    switch(op_index) {
        case 0: buf[expr_len++] = '+'; break;
        case 1: buf[expr_len++] = '-'; break;
        case 2: buf[expr_len++] = '*'; break;
        case 3: buf[expr_len++] = '/'; break;
    }
    gen_space();    // space after operator
}

static void gen_rand_expr() {
    /* Prevent buf overflow: force termination when buffer is nearly full */
    if (expr_len >= 65500) {
        gen_num();
        return;
    }
    switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    expr_len = 0;
    gen_rand_expr();
    buf[expr_len] = '\0';

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr 2>/dev/null");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    int status = pclose(fp);

    /* Filter out runtime errors (division by zero → SIGFPE, etc.) */
    if (ret != 1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) continue;

    printf("%u %s\n", result, buf);
  }
  return 0;
}
