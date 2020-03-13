#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

static int PID = -50;

module_param(PID, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(myint, "Entered PID: \n");

void DFS(struct task_struct *task, int **tree_array, int depth)
{   
    struct task_struct *child;
    struct list_head *list;
    int i = 0;
    int j = 0;
    for(; i<depth; i++) {
        printk(KERN_CONT "-");
    }
    printk(KERN_CONT "PID: <%d>, Creation Time: %li\n", task->pid, task->start_time);
    list_for_each(list, &task->children) {
        child = list_entry(list, struct task_struct, sibling);
        tree_array[depth+1][j] = (int) child->pid;
        DFS(child, tree_array, depth+1);
        j++;
    }
}


/* This function is called when the module is loaded. */
int proc_init(void)
{
    printk(KERN_INFO "Loading Module\n");
    // checking the given PID
    if (PID < 0)
    {
        printk("Wrong PID, going to unload.\n");
        return 1;
    }
    else // valid PID
    {
        struct task_struct *task;
        int **tree_array = kmalloc(100, GFP_KERNEL);
        int i;
        for (i = 0; i < 100; ++i) {
            tree_array[i] = kmalloc(100, GFP_KERNEL);
        }
        // finding the task with given PID
        task = pid_task(find_vpid((pid_t)PID), PIDTYPE_PID);
        tree_array[0][0] = (int) task->pid;
        DFS(task,tree_array,0);
        for (i = 0; i < 100; ++i) {
            kfree(tree_array[i]);
        }
        kfree(tree_array);
        //printk("1. dim size %d, 2. dim size %d",sizeof(tree_array)/sizeof(tree_array[0]),sizeof(tree_array[0])/sizeof(tree_array[0][0]));
    }

    return 0;
}

/* This function is called when the module is removed. */
void proc_exit(void)
{
    printk(KERN_INFO "Removing Module\n");
}
/* Macros for registering module entry and exit points. */
module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Process Module");
MODULE_AUTHOR("Furkan Sahbaz");