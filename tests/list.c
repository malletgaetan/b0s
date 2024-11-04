#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "kernel/lib/list/list.h"

struct test_struct {
	int data;
	struct list_head list;
};

static struct test_struct *create_test_struct(int data)
{
	struct test_struct *item = malloc(sizeof(struct test_struct));
	assert(item && "malloc failed :'[");
	item->data = data;
	INIT_LIST_HEAD(&item->list);
	return item;
}

static void free_list(struct list_head *head)
{
	struct list_head *pos, *n;
	list_for_each_safe(pos, n, head)
	{
		struct test_struct *entry = list_entry(pos, struct test_struct, list);
		list_del(&entry->list);
		free(entry);
	}
}

static void assert_equal(struct list_head *head, int arr[])
{
	u64 i = 0;
	struct list_head *pos;
	list_for_each_ro(pos, head)
	{
		assert(((struct test_struct *)list_entry(pos, struct test_struct, list))->data ==
				   arr[i++] &&
			   "bad insertion order");
	}
}

int main(void)
{
	LIST_HEAD(test_list);

	assert(list_empty(&test_list) && "list should be init empty");

	struct test_struct *item1 = create_test_struct(10);
	struct test_struct *item2 = create_test_struct(20);
	struct test_struct *item3 = create_test_struct(30);

	list_add(&item1->list, &test_list); // add at front
	list_add_tail(&item2->list, &test_list);
	list_add_tail(&item3->list, &test_list);

	assert_equal(&test_list, (int[]){10, 20, 30});
	assert(!list_empty(&test_list) && "list shouldn't be empty");

	list_del(&item2->list);
	free(item2);
	assert_equal(&test_list, (int[]){10, 30});

	struct test_struct *item4 = create_test_struct(40);
	list_add(&item4->list, &test_list); // add at front
	assert_equal(&test_list, (int[]){40, 10, 30});

	free_list(&test_list);

	assert(list_empty(&test_list) && "list should be empty");
}
