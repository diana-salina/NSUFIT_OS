#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void reverse(char* str, char* res, int len) {
	for (int i = 0; i < len; ++i) {
		res[len - i - 1] = str[i];
	}
}

void reverse_file(FILE* in, FILE* out) {
	fseek(in, 0, SEEK_END);
	long size = ftell(in);
	char* buffer = (char*)malloc(sizeof(char)  * 32);
	if (!buffer) {
		printf("malloc error in buffer\n");
	}
	int count = (size % 32) > 0 ? size / 32 + 1 : size / 32;
	while (count > 0) {
		--count;
		fseek(in, 32 * count, SEEK_SET);
		int read_size = (count == size / 32) ? (size % 32) : 32;
		fread(buffer, read_size, 1, in);
		for(int i = read_size - 1; i >= 0; --i) {
			fwrite(&buffer[i], sizeof(char), 1, out);
		}
	}
	free(buffer);
}

void printstr(char* str) {
	for(int i = 0; i < strlen(str); ++i) {
		printf("%c", str[i]);
	}
	printf("\n");
}

int reverse_inside(char *path, char *path_reversed) {
	DIR* dir;
	struct dirent* cur_file;
	if (!(dir = opendir(path))) {
		printf("opening dir error: %s\n", path);
		return 1;
	}
	printf("%s opened\n", path);

	while(cur_file = readdir(dir)) {
		if (cur_file->d_name[0] == '.') continue;

		if (cur_file->d_type == DT_DIR) {
			char *name = cur_file->d_name;
			char* fpath = (char*)malloc(strlen(path) + strlen(name) + 1);
			char* fpath_reversed = (char*)malloc(strlen(path_reversed) + strlen(name) + 1);
			char *fname_reversed = (char*)malloc(strlen(name) + 1);
			if (!fpath || !fpath_reversed || !fname_reversed) {
				printf("realloc error");
				return 1;
			}
			sprintf(fpath, "%s/%s", path, name);
			reverse(name, fname_reversed, strlen(name));
			sprintf(fpath_reversed, "%s/%s", path_reversed, fname_reversed);

			struct stat dir_stat;
			if (stat(fpath, &dir_stat) != 0) {
				printf("connot get st_mode of %s\n", fpath);
				free(fpath);
				free(fpath_reversed);
				free(fname_reversed);
				return 1;
			}

			__mode_t dir_mode = dir_stat.st_mode;

			if(mkdir(fpath_reversed, dir_mode) != 0) {
				printf("mkdir error: %s\n", fpath_reversed);
				free(fpath);
				free(fpath_reversed);
				free(fname_reversed);
				return 1;
			}

			reverse_inside(fpath, fpath_reversed);
			free(fpath);
			free(fpath_reversed);
		} else if(cur_file->d_type == DT_REG) {
			char *name = cur_file->d_name;
			int file_len = strlen(name);
			char* local_path = (char*)malloc(strlen(path) + strlen(name) + 1);
			char* local_path_reversed = (char*)malloc(strlen(path_reversed) + strlen(name) + 1);
			char* lname_reversed = (char*)malloc(1 + strlen(name));
			if (!local_path || !local_path_reversed || !lname_reversed) {
				printf("realloc error");
				return 1;
			}
			reverse(name, lname_reversed, strlen(name));
			sprintf(local_path, "%s/%s", path, name);
			sprintf(local_path_reversed, "%s/%s", path_reversed, lname_reversed);
			printstr(local_path);
			printstr(local_path_reversed);

			FILE* file = fopen(local_path, "rb");
			FILE* file_reversed = fopen(local_path_reversed, "wb");

			if (!file || !file_reversed) {
				printf("file error\n");
				return 1;
			}

			struct stat file_stat;
			if (stat(local_path, &file_stat) != 0) {
				printf("cannot get st_mode of %s\n", local_path);
				return 1;
			}
			mode_t file_mode = file_stat.st_mode;
			if (chmod(local_path_reversed, file_mode) != 0) {
				printf("Error in chmod of %s\n", local_path_reversed);
				return 1;
			}
			printf("files %s %s\n", local_path, local_path_reversed);
			reverse_file(file, file_reversed);

			fclose(file);
			fclose(file_reversed);
			free(local_path);
			free(local_path_reversed);
			free(lname_reversed);
		}
	}
	closedir(dir);
	return 0;
}

int main(int argc, char** argv) {
	if (!argv[1]) {
		printf("no directory name inputed\n");
		return 0;
	}
	char* name_input = (char*)malloc(sizeof(char) * strlen(argv[1]));
	strncpy(name_input, argv[1], strlen(argv[1]));
	int len = strlen(name_input);
	char* name_reversed = (char*)malloc(sizeof(char) * len);
	if (!name_reversed) {
		printf("malloc error\n");
		return 1;
	}
	reverse(name_input, name_reversed, len);

	struct stat dir_stat;
	if (stat(name_input, &dir_stat) != 0) {
			printf("connot get st_mode of %s\n", name_input);
		return 1;
	}
	__mode_t dir_mode = dir_stat.st_mode;

	if(mkdir(name_reversed, dir_mode) != 0) {
		printf("mkdir error\n");
		return 1;
	}

	if (reverse_inside(name_input, name_reversed) == 1) {
		free(name_reversed);
		free(name_input);
		return 1;
	}
	free(name_reversed);
	free(name_input);
	return 0;
}
