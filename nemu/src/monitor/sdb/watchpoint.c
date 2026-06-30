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

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  uint32_t orig_val;
  uint32_t cur_val;

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

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

