#include <unistd.h>
#include <sys/syscall.h>

ssize_t my_syswrite(int fd, const void *buf, size_t count) {
	ssize_t ret = 0;
	__asm__ volatile("syscall" : "=a"(ret) : "a"(SYS_write), "D"(fd), "S"(buf), "d"(count) : );
	return ret;
}

int main(void) {
	my_syswrite(1, "Hello, world!\n", 14);
	return 0;
}

//a - ax
//D - di
//S - si
//d - dx
