#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <limits.h>

int main() {
	int val = 0;
	int pipeAr[2];
	if (pipe(pipeAr) == -1) return 1;

	pid_t reader = fork();

	if (reader == 0) {
		printf("%d\n", getpid());
		close(pipeAr[1]);
		int last = 0;
		while(read(pipeAr[0], &val, sizeof(int))) {
			if (val != ++last) {
				printf("not equal: %d <-> %d\n", val, last);
			} else printf("okey: %d\n", val);
			sleep(1);
		}
	} else if (reader > 0) {
		close(pipeAr[0]);
		while(1){
			val++;
			if (write(pipeAr[1], &val, sizeof(int)) == 0) {
				printf("SWQd\n");
				break;
			}
			if (val == INT_MAX) val = 0;
			//if (val > 10) exit(0);
			sleep(1);
		}
	} else {
		return 1;
	}

	return 0;
}