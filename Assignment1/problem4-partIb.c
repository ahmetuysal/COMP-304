#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>

typedef struct birthday {
    int day;
    int month;
    int year;
    struct list_head list;
};

static LIST_HEAD(birthday_list);

/* This function is called when the module is loaded. */
int simpleInit(void) {
    printk(KERN_INFO "Loading Module Linked List\n");
    struct birthday *person;
    person = kmalloc(sizeof(*person), GFP_KERNEL);
    person->day = 2;
    person->month= 8;
    person->year = 1995;
    INIT_LIST_HEAD(&person->list);
    int i;
    for (i = 0 ; i < 4; i++) {
        struct birthday *person;
        person = kmalloc(sizeof(*person), GFP_KERNEL);
        person->day = 2 + 5 * i;
        person->month= 2 * i ;
        person->year = 1990 + i;
        list_add_tail(&person->list, &birthday_list);
    }

    struct birthday *birthdayPtr;

    list_for_each_entry(birthdayPtr, &birthday_list, list) {
        /* on each iteration ptr points */
        /* to the next birthday struct */
        printk(KERN_INFO "Birthday, day: %d, month: %d, year: %d\n", birthdayPtr->day, birthdayPtr->month, birthdayPtr->year);
    }

    return 0;
}
/* This function is called when the module is removed. */
void simpleExit(void)
{
    printk(KERN_INFO "Removing Module Linked List\n");

    struct birthday *ptr, *next;
    list_for_each_entry_safe(ptr,next,&birthday_list,list) {
        /* on each iteration ptr points */
        /* to the next birthday struct */
        list_del(&ptr->list);
        kfree(ptr);
    }

    printk(KERN_INFO "Linked list elements are deleted\n");
}
/* Macros for registering module entry and exit points. */

module_init(simpleInit);
module_exit(simpleExit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");