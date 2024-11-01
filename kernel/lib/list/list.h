#ifndef LIST_H
#define LIST_H

# include "kernel/types.h"

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list) {
    list->next = list;
    list->prev = list;
}

static inline void __list_add(struct list_head *el, struct list_head *prev, struct list_head *next) {
    next->prev = el;
    el->next = next;
    el->prev = prev;
    prev->next = el;
}

static inline void list_add(struct list_head *el, struct list_head *head) {
    __list_add(el, head, head->next);
}

static inline void list_add_tail(struct list_head *el, struct list_head *head) {
    __list_add(el, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next) {
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline int list_empty(const struct list_head *head) {
    return head->next == head;
}

#define list_entry(ptr, type, member) ({ \
    const typeof(((type *)0)->member) *__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type, member));})

// safe for read only
#define list_for_each_ro(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

// safe for deleting element
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

#endif