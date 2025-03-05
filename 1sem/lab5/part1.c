#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int global_var = 10;

int main() {
	printf("PID: %d\n", getpid());
	//printf("watch -n 1 /proc/%d/maps\n", getpid());

	int local_var = 10;
	printf("local var: %p -> %d\n", &local_var, local_var);
	printf("global var: %p -> %d\n", &global_var, global_var);
	//sleep(10);
	pid_t pid = fork();

	if (pid == 0) {
		printf("CHILD PID: %d\n", getpid());
		//printf("watch -n 1 /proc/%d/maps\n", getpid());
		//printf("ps -p %d -o pid,ppid\n", getpid());

		printf("parent pid: %d\n", getppid());
		printf("local var in child: %p -> %d\n", &local_var, local_var);
		printf("global var in child: %p -> %d\n", &global_var, global_var);
		global_var = 1;
		local_var = 2;
		printf("local var in child after change: %p -> %d\n", &local_var, local_var);
		printf("global var in child after change: %p -> %d\n", &global_var, global_var);
		sleep(15);
		exit(5);
	} else if (pid > 0) {
		printf("local var in parent: %p -> %d\n", &local_var, local_var);
		printf("global var in parent: %p -> %d\n", &global_var, global_var);
		sleep(20);
		printf("local var in parent in time: %p -> %d\n", &local_var, local_var);
		printf("global var in parent in  time: %p -> %d\n", &global_var, global_var);
	//exit(0);
	/*
	int status;
	waitpid(pid, &status, 0);
	if (WIFEXITED(status)) {
		printf("child process exit code: %d\n", WEXITSTATUS(status));
	} else printf("child process exited incorrectly\n");
	*/
	} else {
		printf("fork error\n");
		return 1;
	}

	return 0;
}
