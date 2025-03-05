#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int global_iniz = 10;
int global_uniniz;
const int const_global = 15;

void print_local() {
	int var = 1;
	printf("Address of local: %p\n", (void*)&var);
}

void print_static() {
	static int var = 1;
	printf("Address of static: %p\n", (void*)&var);
}

void print_const() {
	const int var = 1;
	printf("Address of const: %p\n", (void*)&var);
}

int* get_pointer() {
	int point = 10;
	printf("Point address in fanc: %p\n", (void*)&point);
	int* answ = &point;
	return answ;
}

void hello_world() {
	char* buffer = (char*)malloc(sizeof(char) * 100);
	strcpy(buffer, "Hello, world!");
	printf("In buffer: %s\n", buffer);
	free(buffer);
	printf("In buffer after free: %s\n", buffer);

	char* buffer_2 = (char*)malloc(sizeof(char) * 100);
	strcpy(buffer_2, "Hello, second world!");
	printf("In  second buffer: %s\n", buffer_2);
	buffer_2 += sizeof(buffer) / 2;
	//free_point(buffer_2);
	free(buffer_2);
	printf("in second buffer after special free: %s\n", buffer_2);
}

void get_env() {
	char* env_var = getenv("AAA_PLISS_HELP");
	if (env_var) printf("AAA_PLISS_HELP value: %s\n", env_var);
	else {
		printf("AAA_PLISS_HELP do not exist\n");
		return;
	}

	//if (setenv("AAA_PLISS_HELP", "godwon'thelp", 1) == 0) {
	if (putenv("AAA_PLISS_HELP=god_won't_help") != 0) {
		printf("AAA_PLISS_HELP error %d\n", errno);
		return;
	}
	env_var = getenv("AAA_PLISS_HELP");
	if (env_var) {
		printf("AAA_PLISS_HELP new value: %s\n", env_var);
		printf("Addr: %p\n", &env_var);
	}
}

int main() {
	pid_t pid = getpid();
	printf("PID: %d\n", pid);
	//sleep(20);
	print_local();
	print_static();
	print_const();

	printf("Address of global initialized: %p\n", (void*)&global_iniz);
	printf("Address of global uninitialized: %p\n", (void*)&global_uniniz);
	printf("Address of const global: %p\n", (void*)&const_global);

	int* point = get_pointer();
	printf("Point address: %p\n", point);
	printf("Point value: %d\n", *point);

	//hello_world();
	get_env();
	sleep(1000);
}