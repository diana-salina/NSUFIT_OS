#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

void funk() {
	char* buf[4096];
	funk();
}

void signal_handler(int signal_num){
    printf("Segmentation fault catched\n");
    exit(1);
}

int main(int argc, char** argv) {
	printf("PID: %d\n", getpid());
	//sleep(15);
	//funk();

	/*
	for (int i = 0; i < 1000; ++i) {
		char* buf = (char*)malloc(4096 * 4096);
		sleep(1);
		//free(buf);
	}
	*/
	sleep(12);

	signal(SIGSEGV, signal_handler);
	void* region = mmap(NULL, 10 * sysconf(_SC_PAGE_SIZE), PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (region == MAP_FAILED) {
        printf("mmap error\n");
        return 1;
    }
    sleep(3);

    char* try_to_write = "try to write";
    strcpy((char*) region, try_to_write);
    printf("region: %s\n", (char*) region);
    sleep(3);

	char try_to_read = *((char*)region);
    if(try_to_read != -1) printf("region read: %s\n", (char*) region);

    sleep(7);

    if (munmap(region + 4 * sysconf(_SC_PAGE_SIZE), 3 * sysconf(_SC_PAGE_SIZE)) == -1) {
    	printf("munmap error\n");
    	return 1;
    }
    sleep(1000);


	return 0;
}