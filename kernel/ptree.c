#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/prinfo.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/string.h>
#include <../arch/x86/include/asm/uaccess.h>

#define MAX_COMM 128

static int find_next_sibling(struct task_struct *task){
	if(task->sibling.next == &(task->real_parent->children))
		return 0;
	return list_entry(task->sibling.next, struct task_struct, sibling)->pid;
}

static void take_proc_info(struct task_struct *task, struct prinfo *proc, struct prinfo *end_proc){
	if (proc<end_proc){
		proc->state = task->state;
		proc->pid = task->pid;
		proc->parent_pid = task->real_parent->pid;
		
		if(list_empty(&task->children))
			proc->first_child_pid=0;
		else
			proc->first_child_pid=list_entry(task->children.next, struct task_struct, sibling)->pid;
		
		proc->next_sibling_pid = find_next_sibling(task);
		proc->uid = task_uid(task).val;
		//printk("commsize:%ld\n", sizeof(task->comm));
		//get_task_comm(proc->comm, task); // size error 
		strncpy(proc->comm, task->comm, MAX_COMM);
	}
}

static int ptree_traversal(struct task_struct *task, struct prinfo *proc, struct prinfo *end_proc){
        int n=0;
        struct list_head *ptr;
        struct task_struct *temp;

        if(task != &init_task){
                n = 1;
                take_proc_info(task, proc, end_proc);
        }
                list_for_each(ptr, &task->children){
                        temp = list_entry(ptr, struct task_struct, sibling);
                        n += ptree_traversal(temp, proc+n, end_proc);
                }
                return n;
}

SYSCALL_DEFINE2(ptree, struct prinfo *, buf, int *, nr){
	
	int list_size;
	int error;
	int proc_num;
	int val;
	int remain_size;
	void *kbuf, *ubuf, *dealloc_buf;

	if(buf==NULL || nr==NULL)
		return -EINVAL;

	if((error=get_user(list_size, nr))<0)
		return error;

	if(list_size<=0)
		return -EINVAL;

	if(access_ok(buf, list_size * sizeof(struct prinfo))==0)
		return -EFAULT;

	kbuf = kmalloc(list_size*sizeof(struct prinfo), GFP_KERNEL);
	dealloc_buf = kbuf;

	if(kbuf==NULL)
		return -ENOSPC;

	read_lock(&tasklist_lock);
	proc_num = ptree_traversal(&init_task, kbuf, kbuf+list_size*sizeof(struct prinfo));
	read_unlock(&tasklist_lock);

	if(proc_num<0)
		return proc_num;
	if(list_size>proc_num)
		list_size = proc_num;

	ubuf = buf;
	remain_size = list_size*sizeof(struct prinfo);

	kbuf += remain_size;
	ubuf += remain_size;

	while(1){
		ubuf -= remain_size;
		kbuf -= remain_size;
		remain_size = copy_to_user(ubuf, kbuf, remain_size);
		if(remain_size<=0)
			break;
	}

	if(remain_size<0)
		val = remain_size;

	if((error=put_user(list_size, nr))<0)
		val = error;

	kfree(dealloc_buf);
	return val;

}




