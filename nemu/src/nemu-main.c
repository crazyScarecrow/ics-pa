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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

#if 0
extern word_t expr(char *e, bool *success);
extern void init_regex(void);
static void verify_express()
{
    init_regex();
    char ch = '0';
    char buf[30720] = { 0 };
    int index = 0;
    bool is_success = false;
    uint32_t expr_ret = 0;
    char* result = NULL;
    char *file_path = "/home/snow/ics-pa/nemu/tools/gen-expr/build/input";
    FILE* fp = fopen(file_path, "r");
    assert(NULL != fp);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch != '\n') {
            buf[index++] = ch;
        } else {
            result = strstr(buf, " ");
            if (result) {
                buf[result-buf] = '\0';
                expr_ret = strtoul(buf, NULL, 0);
                if (expr_ret == expr(buf + (result-buf + 1), &is_success)) {
                    Log("check success\n");
                } else{
                    Warning("\texpress is %s\n", buf + (result-buf + 1));
                }
            }
            memset(buf, 0, 30720);
            index = 0;
        }
    }
}
#endif
int main(int argc, char *argv[]) {
    //verify_express();
    //return 0;
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
