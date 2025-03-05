#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>

int main() {
	int* reg =  mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (reg == MAP_FAILED) {
		printf("cannot create region\n");
		return 1;
	}

	*reg = 0;

	pid_t reader = fork();

	if (reader == 0) {
		int last = 0;
		while(1) {
			int val = *reg;
			if (val != ++last) {
				printf("not equal: %d <-> %d\n", val, last);
			} else printf("okey: %d\n", val);
			//sleep(1);
			//if (last > 10) exit(0);
		}
	} else if (reader > 0) {
		while(1){
			(*reg)++;
			if (*reg == INT_MAX) *reg = 0;
			//sleep(1);
		}
	} else {
		return 1;
	}

	return 0;
}
