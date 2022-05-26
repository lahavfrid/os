#include <linux/kernel.h>
#include <linux/sched.h>

asmlinkage long sys_hello(void)
{
    printk("Hello, World!\n");
    return 0;
}

asmlinkage long sys_set_status(long status)
{
    if (status != 0 && status != 1)
        return -EINVAL;
    current->faculty = status;
    return 0;
}

asmlinkage long sys_get_status(void)
{
    return current->faculty;
}

asmlinkage long sys_register_process(void)
{

    current->recognized = 1;
    return 0;
}

long get_pid_sum_aux(struct task_struct* p, bool* r)
{
    long sum = 0;
    struct task_struct* iterator;
    struct list_head* head;
    if (p->recognized)
    {
        *r = false;
        if (p->faculty == 0)
        {
            sum += p->pid;
        }
    }
    if (list_empty(&p->children))
    {
        return sum;
    }
    list_for_each(head, &p->children){
        iterator = list_entry(head, struct task_struct, sibling);
        sum += get_pid_sum_aux(iterator, r);
    }
    
    return sum;
}

asmlinkage long sys_get_all_cs(void)
{
    bool nodata = true;
    long sum = 0;
    pid_t init_pid = 1;
    struct task_struct* init = get_pid_task(init_pid, PIDTYPE_PID);
    sum = get_pid_sum_aux(init, &nodata);
    if (nodata)
    {
        return -ENODATA;
    }
    return sum;
}