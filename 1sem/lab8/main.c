#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void catFile(FILE* file) {
	char ch;
	while ((ch = fgetc(file)) != EOF) {
		putchar(ch);
	}
}

int main() {
	char *fileName = "file.txt";
	FILE *file = fopen(fileName, "r");
	if (!file) {
		perror("cannot open file");
		return 1;
	}
	catFile(file);
	fclose(file);

	printf("\n real UID: %d\n", getuid());
	printf("\n effective UID: %d\n", geteuid());

	return 0;
}

