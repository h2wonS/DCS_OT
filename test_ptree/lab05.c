#include <stdio.h>
#include <stdlib.h>

int main(){
	int ret;
	ret = syscall(548);
	printf("%d\n", ret);
	return 0;
}
