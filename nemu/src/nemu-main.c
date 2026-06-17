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

static void verify_express()
{
    char ch = '0';
    char buf[10240] = { 0 };
    int index = 0;
    char* result = NULL;
    char *file_path = "/home/snow/ics-pa/nemu/tools/gen-expr/build/input";
    FILE* fp = fopen(file_path, "r");
    assert(NULL != fp);
    while ((ch = fgetc(fp)) != EOF) {
        if (ch != '\n') {
            buf[index++] = ch;
        } else {
            result = strtok(buf, " ");
            if (!result) {
                printf("result is %s\n", result);
            }
            memset(buf, 0, 4096);
            index = 0;
        }
    }
}

int main(int argc, char *argv[]) {
    verify_express();
    return 0;
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
