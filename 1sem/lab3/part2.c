#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "fcntl.h"
#include "unistd.h"

void print_help() {
	printf("./create_dir <name> ~ create new directory \"name\"\n");
	printf("./ls_dir <name> ~ print a list of \"name\" directory\n");
	printf("./delete_dir <name> ~ delete a directory \"name\"\n"); //
	printf("./create_file <name> ~ create new file \"name\"\n");
	printf("./cat_file <name> ~ print what is inside \"name\" file\n");
	printf("./delete_file <name> ~ delete file \"name\"\n"); //
	printf("./create_slink <target> <name> ~ create symlink \"name\"\n");
	printf("./print_slink <name> ~ print symlink \"name\" content\n");
	printf("./print_file_slink <name> ~ print what is inside \"name\" symlink file\n");
	printf("./delete_slink <name> ~ delete symlink \"name\"\n"); //
	printf("./create_hlink <target> <name> ~ create hardlink \"name\"\n");
	printf("./delete_hlink <name> ~ delete hardlink \"name\"\n");
	printf("./file_info <name> ~ print access rights of file \"name\" and hardlinks for it\n");
	printf("./change_mod <name> <new_mode> ~ change access rights for \"name\" file to <new_mode>\n");
}

int create_dir(char* name) {
	if(mkdir(name, 0777) != 0) {
		printf("mkdir error\n");
		return 1;
	}
	return 0;
}

int ls_dir(char* name) {
	DIR* dir;
	struct dirent* cur_file;
	if (!(dir = opendir(name))) {
		printf("opening dir error\n");
		return 1;
	}
	while (cur_file = readdir(dir)) {
		if (cur_file->d_name[0] == '.') continue;
		printf("%s\n", cur_file->d_name);
	}
	return 0;
}

int delete_dir(char* name) {
	if(rmdir(name) != 0) {
		printf("rmdir error\n");
		return 1;
	}
	return 0;
}

int create_file(char* name) {
	FILE* f = fopen(name, "w");
	if (!f) {
		printf("cannot create file %s\n", name);
		return 1;
	}
	fclose(f);
	return 0;
}

int cat_file(char* name) {
	int f = open(name,  O_RDONLY);
	if (!f) {
		printf("cannot open file %s\n", name);
		return 1;
	}

	char buffer[256];
	ssize_t bytes;
	while((bytes = read(f, buffer, sizeof(buffer))) > 0) {
		if (write(STDOUT_FILENO, buffer, bytes) != bytes) {
			printf("printing file error %s\n", name);
			return 1;
		}
	}
	if (bytes == -1) {
		printf("reading file error %s\n", name);
		return 1;
	}

	close(f);
	return 0;
}

int delete_file(char* name) {
	if (remove(name) != 0) {
		printf("deleting %s file error\n", name);
		return 1;
	}

	return 0;
}

int create_slink(char* name, char* link) {
	if (symlink(name, link) != 0) {
		printf("creating symlink error: %s %s\n", name, link);
		return 1;
	}
	return 0;
}

int print_slink(char* name) {
	char buffer[PATH_MAX];
	ssize_t bytes = readlink(name, buffer, sizeof(buffer));
	if (bytes == -1) {
		printf("reading %s symlink error\n", name);
		return 1;
	}
	buffer[bytes] = '\0';
	printf("%s\n", buffer);

	return 0;
}

int print_file_slink(char* name) {
	char buffer[PATH_MAX];
	ssize_t bytes = readlink(name, buffer, sizeof(buffer));
	if (bytes == -1) {
		printf("reading %s symlink error\n", name);
		return 1;
	}
	buffer[bytes] = '\0';

	return cat_file(buffer);
}

int delete_slink(char* name) {
	if (unlink(name) != 0) {
		printf("cannot delete %s symlink\n", name);
		return 1;
	}
	return 0;
}

int create_hlink(char* target, char* name) {
	if (link(target, name) != 0) {
		printf("cannot create %s hardlink\n", name);
		return 1;
	}
	return 0;
}

int delete_hlink(char* name) {
if (unlink(name) != 0) {
		printf("cannot delete %s hardlink\n", name);
		return 1;
	}
	return 0;
}

void get_str_mode(mode_t mode, char* str) {
	for(int i = 0; i < 9; ++i) {
        str[i] = '-';
    }

	if (S_ISDIR(mode)) {
		str[0] = 'd';
    } else if (S_ISLNK(mode)) str[0] = 'l';

    if ((mode & S_IRUSR) != 0) str[1] = 'r';
    if ((mode & S_IWUSR) != 0) str[2] = 'w';
    if ((mode & S_IXUSR) != 0) str[3] = 'x';
    if ((mode & S_IRGRP) != 0) str[4] = 'r';
    if ((mode & S_IWGRP) != 0) str[5] = 'w';
    if ((mode & S_IXGRP) != 0) str[6] = 'x';
    if ((mode & S_IROTH) != 0) str[7] = 'r';
    if ((mode & S_IWOTH) != 0) str[8] = 'w';
    if ((mode & S_IXOTH) != 0) str[9] = 'x';
}

int file_info(char* name) {
	struct stat info;
	if (stat(name, &info) != 0) {
		printf("cannot get %s info\n", name);
		return 1;
	}
	char* mode = (char*)malloc(sizeof(char)  *  10);
	get_str_mode(info.st_mode, mode);
	printf("Mode: %s\n", mode);
	printf("Hurdlinks amount: %lu\n", info.st_nlink);
	free(mode);
	return 0;
}

int get_mode_from_str(mode_t* mode, char* str) {
    for(int i = 0; i < 10; ++i) {
        if(str[i] == 'r') *mode |= S_IRUSR | S_IRGRP | S_IROTH;
        else if(str[i] == 'w') *mode |= S_IWUSR | S_IWGRP | S_IWOTH;
        else if(str[i] == 'x') *mode |= S_IXUSR | S_IXGRP | S_IXOTH;
        else if(str[i] == '-') continue;
        else {
            printf("invalid mode symbol: %d, %c\n", i, str[i]);
            return 1;
        }
    }
	return 0;
}

int change_mod(char* name, char* new_mode) {
	//char* tmp = (char*)malloc(10);
	//strcpy(tmp, new_mode);
	mode_t mode = 0;
	if (get_mode_from_str(&mode, new_mode) != 0) {
		return 1;
	}
	if (chmod(name, mode) != 0) {
		printf("cannot set new mode\n");
		return 1;
	}

	return 0;
}

int execute(int argc, char** argv) {
	if (strcmp(argv[1], "help") == 0) {
		print_help();
	} else if (strcmp(argv[0], "./create_dir") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (create_dir(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./ls_dir") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (ls_dir(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./delete_dir") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (delete_dir(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./create_file") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (create_file(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./cat_file") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (cat_file(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./delete_file") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (delete_file(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./create_slink") == 0) {
		if (argc != 3) {
			printf("invalid input\n");
			return 1;
		}
		if (create_slink(argv[1], argv[2]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./print_slink") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (print_slink(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./print_file_slink") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (print_file_slink(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./delete_slink") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (delete_slink(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./create_hlink") == 0) {
		if (argc != 3) {
			printf("invalid input\n");
			return 1;
		}
		if (create_hlink(argv[1], argv[2]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./delete_hlink") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (delete_hlink(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./file_info") == 0) {
		if (argc != 2) {
			printf("invalid input\n");
			return 1;
		}
		if (file_info(argv[1]) != 0) {
			return 1;
		}
	} else if (strcmp(argv[0], "./change_mod") == 0) {
		if (argc != 3) {
			printf("invalid input\n");
			return 1;
		}
		if (change_mod(argv[1], argv[2]) != 0) {
			return 1;
		}
	} else {
		printf("aaaaaa\n");
		return 1;
	}
	return 0;
}


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("incorrect input\n");
		printf("run \"./go help\" to see command list\n");
		return 1;
	}

	if (execute(argc, argv) != 0) {
		printf("execute error\n");
		return 1;
	}

	return 0;
}
