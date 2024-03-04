#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/**
 * node_get_val() - Get the element value from provied node.
 * @node: list of element_t
 *
 * Return: the element value
 */
static inline char *node_get_val(const struct list_head *node)
{
    if (!node)
        return NULL;

    return list_entry(node, element_t, list)->value;
}

/**
 * node_get_val() - Swap the element's value of provide nodes
 * @n1: list of element_t
 * @n2: list of element_t
 */
static inline void node_swap_val(struct list_head *n1, struct list_head *n2)
{
    if (!n1 || !n2 || n1 == n2)
        return;

    element_t *e1 = list_entry(n1, element_t, list);
    element_t *e2 = list_entry(n2, element_t, list);

    char *tmp = e1->value;
    e1->value = e2->value;
    e2->value = tmp;
}

/**
 * q_create_element() - Create the element by provided string value
 * @sp: value of element to be created
 *
 * This function is intended for internal use only.
 */
static inline element_t *q_create_element(const char *sp)
{
    element_t *new_ele = test_malloc(sizeof(element_t));
    if (!new_ele)
        return NULL;

    new_ele->value = test_strdup(sp);
    if (!new_ele->value) {
        test_free(new_ele);
        return NULL;
    }

    return new_ele;
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = test_malloc(sizeof(struct list_head));
    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);

    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }
    test_free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_ele = q_create_element(s);
    if (!new_ele)
        return false;

    list_add(&new_ele->list, head);

    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new_ele = q_create_element(s);
    if (!new_ele)
        return false;

    list_add_tail(&new_ele->list, head);

    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_first_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, target->value, bufsize - 1);
        sp[bufsize - 1] = 0;
    }

    list_del_init(&target->list);

    return target;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *target = list_last_entry(head, element_t, list);

    if (sp) {
        strncpy(sp, target->value, bufsize - 1);
        sp[bufsize - 1] = 0;
    }

    list_del(&target->list);

    return target;
}

int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int len = 0;
    struct list_head *node;

    list_for_each (node, head) {
        len++;
    }

    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head *slow = head;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;

    q_release_element(q_remove_head(slow, NULL, 0));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head)
        return false;

    struct list_head **indir = &head->next;
    while (*indir != head) {
        bool has_dup = false;
        while ((*indir)->next != head &&
               !strcmp(node_get_val(*indir), node_get_val((*indir)->next))) {
            has_dup = true;
            q_release_element(q_remove_tail((*indir)->next, NULL, 0));
        }

        if (has_dup) {
            q_release_element(q_remove_tail((*indir)->next, NULL, 0));
        }

        indir = &(*indir)->next;
    }

    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head **indir = &head->next;

    while (*indir != head && (*indir)->next != head) {
        node_swap_val(*indir, (*indir)->next);
        indir = &(*indir)->next->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *fw_iter = head->next;
    struct list_head *bw_iter = head->prev;

    while (fw_iter != bw_iter && fw_iter != bw_iter->next) {
        node_swap_val(fw_iter, bw_iter);
        fw_iter = fw_iter->next;
        bw_iter = bw_iter->prev;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head) || k <= 1)
        return;

    struct list_head *fw_iter, *bw_iter, *next_start = head->next;

    for (;;) {
        fw_iter = bw_iter = next_start;

        for (int count = 0; count++ < k - 1;) {
            bw_iter = bw_iter->next;
            if (bw_iter == head)
                return;
        }
        next_start = bw_iter->next;

        while (fw_iter != bw_iter && fw_iter != bw_iter->next) {
            node_swap_val(fw_iter, bw_iter);
            fw_iter = fw_iter->next;
            bw_iter = bw_iter->prev;
        }
    }
}

static struct list_head *mergeTwoLists(struct list_head *l1,
                                       struct list_head *l2,
                                       bool descend)
{
    struct list_head *sorted = NULL, **indir_iter = &sorted;
    for (;;) {
        bool cmp_result = descend
                              ? strcmp(node_get_val(l1), node_get_val(l2)) > 0
                              : strcmp(node_get_val(l1), node_get_val(l2)) < 0;

        if (cmp_result) {
            *indir_iter = l1;
            indir_iter = &l1->next;
            l1 = l1->next;
            if (!l1) {
                *indir_iter = l2;
                break;
            }
        } else {
            *indir_iter = l2;
            indir_iter = &l2->next;
            l2 = l2->next;
            if (!l2) {
                *indir_iter = l1;
                break;
            }
        }
    }

    return sorted;
}

static struct list_head *merge_lists(struct list_head **lists,
                                     int low,
                                     int high,
                                     bool descend)
{
    if (low > high)
        return NULL;
    if (low == high)
        return lists[low];

    int mid = (low + high) / 2;
    struct list_head *left = merge_lists(lists, low, mid, descend);
    struct list_head *right = merge_lists(lists, mid + 1, high, descend);
    return mergeTwoLists(left, right, descend);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!(head) || list_empty(head) || list_is_singular(head))
        return;

    size_t count = 0;
    struct list_head *lists[100000], *sorted = head->next;

    while (sorted != head) {
        struct list_head *iter = sorted;
        while (iter != head && iter->next != head &&
               strcmp(node_get_val(iter), node_get_val(iter->next)) <= 0) {
            iter = iter->next;
        }

        lists[count++] = sorted;
        sorted = iter->next;
        iter->next = NULL;
    }

    head->next = merge_lists(lists, 0, count - 1, descend);
    sorted = head;

    while (sorted->next) {
        sorted->next->prev = sorted;
        sorted = sorted->next;
    }

    sorted->next = head;
    head->prev = sorted;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int count = 1;
    char *min_val = node_get_val(head->prev);
    struct list_head *bw_iter = head->prev;

    while (bw_iter->prev != head) {
        if (strcmp(node_get_val(bw_iter->prev), min_val) > 0) {
            q_release_element(q_remove_tail(bw_iter, NULL, 0));
        } else {
            count++;
            min_val = node_get_val(bw_iter->prev);
            bw_iter = bw_iter->prev;
        }
    }

    return count;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int count = 1;
    char *max_val = node_get_val(head->prev);
    struct list_head *bw_iter = head->prev;

    while (bw_iter->prev != head) {
        if (strcmp(node_get_val(bw_iter->prev), max_val) < 0) {
            q_release_element(q_remove_tail(bw_iter, NULL, 0));
        } else {
            count++;
            max_val = node_get_val(bw_iter->prev);
            bw_iter = bw_iter->prev;
        }
    }

    return count;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head)
        return 0;

    queue_contex_t *q2, *q1 = list_entry(head->next, queue_contex_t, chain);
    struct list_head *q1_node, *q2_node, *q_src_node = head->next->next;

    while (q_src_node != head) {
        for (q2 = list_entry(q_src_node, queue_contex_t, chain);
             q2->q->next != q2->q;) {
            q1_node = q1->q->next, q2_node = q2->q->next;
            while (q1_node != q1->q &&
                   strcmp(node_get_val(q1_node), node_get_val(q2_node)) < 0) {
                q1_node = q1_node->next;
            }

            list_del(q2_node);
            list_add_tail(q2_node, q1_node);
        }

        q1->size += q2->size;
        q2->size = 0;
        q_src_node = q_src_node->next;
    }

    return q1->size;
}
