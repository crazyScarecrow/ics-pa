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

#include "sdb.h"

#define NR_WP 32
#define WP_NAME_LEN 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t orig_val;
  uint32_t cur_val;
  char wp_name[WP_NAME_LEN];

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
static int wp_idx = 1;

WP* new_wp()
{
    WP* temp = NULL;
    if (NULL == free_) {
        Warning("There is no more watch pointer resource");
        assert(0);
        return NULL;
    }
    /* remove the wp from free_ */
    temp = free_;
    temp->next = NULL;
    free_ = free_->next;

    /* add the wp to the head */
    if (NULL == head) {
        head = temp;
    } else {
       temp->next = head->next;
       head->next = temp;
    }

    temp->NO = wp_idx++;

    return temp;
}

void free_wp(WP *wp_pointer)
{
    WP *cur = NULL, *prev = NULL;
    if (NULL == wp_pointer) {
        Warning("Invalid watch pointer");
        return;
    }

    /* remove the ep from head */
    if (head == wp_pointer) {
        head = head->next;
    } else {
        cur = head;
        while (NULL != cur) {
            if (cur == wp_pointer) {
                prev->next = cur->next;
                cur->next = NULL;
                break;
            }
            prev = cur;
            cur = cur->next;
        }
    }

    cur->next = free_->next;
    free_ = cur;
    return;
}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void show_wp_info() {
    WP* temp = NULL;
    if (NULL == head) {
        printf("No watchpoints.\n");
    } else {
        printf("No.        What\n");
        temp = head;
        while (NULL != temp) {
           printf("%d        %s\n", temp->NO, temp->wp_name);
           temp = temp->next;
        }
    }
}

extern word_t expr(char * e, bool * success);

void create_wp(char *args) {
    WP *wp = NULL;
    bool is_success = false;;

    if (NULL == args) {
        Warning("Invalid args\n");
        return;
    }

    wp = new_wp();
    if (NULL == wp) {
        Warning("There is no more watch point\n");
        return;
    }

    memset(wp->wp_name, 0, WP_NAME_LEN);
    strncpy(wp->wp_name, args, strlen(args));
    wp->orig_val = expr(args, &is_success);

    return;
}

void del_wp(uint32_t wp_no) {
    WP* temp = NULL;
    if (NULL == head) {
        Warning("There is no watch point");
        return;
    }

    temp = head;
    while (NULL != temp) {
        if (wp_no == temp->NO) {
            break;
        }
        temp = temp->next;
    }
    free_wp(temp);

    return;
}
