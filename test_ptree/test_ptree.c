#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

struct prinfo {
  int64_t state;             /* current state of process */
  pid_t pid;              /* process id */
  pid_t parent_pid;       /* process id of parent */
  pid_t first_child_pid;  /* pid of oldest child */
  pid_t next_sibling_pid; /* pid of younger sibling */
  int64_t uid;               /* user id of process owner */
  char comm[64];          /* name of program executed */
};


void print_ptree(struct prinfo *p, int i, int tab){
	int j;
	for(j=0;j<tab;j++)
		printf("   ");
	printf("%s,%d,%ld,%d,%d,%d,%ld\n", p[i].comm, p[i].pid, p[i].state, p[i].parent_pid, p[i].first_child_pid, p[i].next_sibling_pid, p[i].uid);
}

int main(int argc, char* argv[]) {
	struct prinfo *proc = NULL;
	int tab=0;
	if (argc<2)
		fprintf(stderr, "insert the ptree size!\n");
	int size = atoi(argv[1]);
	int re, i;
	
	int stack[size];
	int p=0;
	
	while(1){		
		proc = (struct prinfo*)malloc(size * sizeof(struct prinfo));
		re = syscall(548, proc, &size);
		if(re == size) 
			break;
		size = re + 10;
		free(proc);
	}
	

	for(i=0; i<re; i++){
		
		print_ptree(proc, i, tab);
		
		if(proc[i].next_sibling_pid !=0){
			stack[p]=tab;
			p++;
		}
		if(proc[i].first_child_pid !=0)
			tab++;
		else{
			p--;
			tab = stack[p];
		}

	}
	free(proc);
	return 0;
}
