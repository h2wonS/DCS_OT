#include <linux/syscalls.h>
#include <linux/kernel.h>

SYSCALL_DEFINE0(mycall){
	printk("here is mycall\n");
	return 11;
}
