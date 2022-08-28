#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// There are my comments and suggested changes to the code. Commented code is original code from test task

// 1. On 64-bit platforms list_t will take 16 bytes of memory. The sum of structure's elements is 12 bytes. We can save 4 bytes by structure padding disabling. Allignment required in case of list_t array, none for linked list.
// 2. Just add "#pragma pack(1)" before list_s typedef
typedef struct list_s
{
    struct list_s *next; /* NULL for the last item in a list */
    int data;
}
list_t;

/* Counts the number of items in a list. */
int count_list_items(const list_t *head)
{
    const int MAX_SIZE = 100;
    int listSize = 0;

    if (!head)
        return listSize;

    while (head && (listSize < MAX_SIZE))
    {
        head = head->next; listSize++;
    }

    return listSize;

    //if (head->next)
    //{
    //    return count_list_items(head->next) + 1;
    //}
    //else
    //{
    //    return 1;
    //}
}
// 1. Recursive invocation of count_list_items function will cause stack overflow on too long lists
// 2. Have no limit with list's maximum size because need an additional way to stop walking through the list
// 3. No check for head pointer for NULL (repeats in the next functions). The segmentation fault can be caused while dereferencing of "next" field if head pointer is NULL


/* Inserts a new list item after the one specified as the argument. */
void insert_next_to_list(list_t *item, int data)
{
    if (item == NULL) return;

    list_t *newItem = (list_t*)malloc(sizeof(list_t));
    newItem->next = item->next;
    newItem->data = data;

    item->next = newItem;

    //return;
    //(item->next = (list_t*)malloc(sizeof(list_t)))->next = item->next;
    //item->next->data = data;
}
// 1. No casting to (list_t*) after memory allocating for a new node. Few compilers can fail compilation of this code
// 2. The function have potential undefined behavior and written tricky that can mislead


/* Removes an item following the one specificed as the argument. */
void remove_next_from_list(list_t *item)
{
    if (!item) return;

    if (item->next)
    {
        list_t *nextItem = item->next;

        item->next = item->next->next;
        free(nextItem);
    }

    //if (item->next)
    //{
    //    item->next = item->next->next;
    //    free(item->next);
    //}
}
// 1. This code will crash the application with segmentation fault error or to undefined behavior because we're freeing the memory before usage
// 2. To exclude the next item from list we need to save the pointer to the element for removal


/* Returns item data as text. */
// char *item_data(const list_t *list)
// {
//     char buf[12];
//     sprintf(buf, "%d", list->data);
//
//     return buf;
// }
// 1. Buffer overflow situation. Returning of the local variable that allocated on stack frame will cause undefined behavior. Dereferencing and modification of returned poiner can damage local variables, parameters, and return addresses of another functions
// 2. Name of function misleading. The function doesn't return item data. Its' just printing item data to as signed decimal integer.
// 3. It's better to rename "list" parameter to "item" because the function prints only one item. No all list items.

// First solution
char *item_data(const list_t *list)
{
    static char buf[12];
    sprintf(buf, "%d", list->data);

    return buf;
}
// Just make the bufer as static. Return pointer points to the ".data" section instead of stack frame
// But data by this pointer will be relevant until next call of the function
// Disadvantages of this solution can be solved by the second solution

// Second solution
int print_item_data(const list_t *list, char* printBuf)
{
    if (!list || !printBuf)
        return -1;

    return sprintf(printBuf, "%d", list->data);
}


//////////////////////////////////////


void PrintTheList(list_t *head)
{
    if (!head)
        return;

    for (; head; head = head->next)
    {
        printf("%i ", head->data);
    }

    putchar('\n');
}

void PushBack(list_t *head, int data)
{
    if (!head)
        return;

    while (head->next)
        head = head->next;

    list_t *newNode = (list_t*)malloc(sizeof(list_t));
    newNode->data = data;
    newNode->next = NULL;

    head->next = newNode;
}

int main()
{
    list_t lst = { NULL, -2147483647 };

    PushBack(&lst, 10);
    PushBack(&lst, 2);
    PushBack(&lst, 3);
    PushBack(&lst, 4);
    PushBack(&lst, 5);

    insert_next_to_list(&lst, 50);

    remove_next_from_list(&lst);

    PrintTheList(&lst);
    printf("List length: %i\n", count_list_items(&lst));

	char* itemStr = item_data(&lst);
	printf("item_data print result: %s\n", itemStr);
	
	char buffer[12] = {0};
	print_item_data(&lst, buffer);
	printf("print_item_data print result: %s\n", itemStr);

    return 0;
}
