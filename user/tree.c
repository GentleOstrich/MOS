#include <lib.h>

int tree(char *path, int deep, int *isEnd);
void print(int deep, int *isEnd);

int tree(char *path, int deep, int *isEnd) {
	int fd;
	struct File f, fnext;
	int r,n;
	int nowEnd = 0;
	char next_path[MAXPATHLEN];
	if ((fd = open(path, O_RDONLY)) < 0 ) {
		debugf("open %s failed", path);
		return fd;
	}
	if (deep == 0) {
		if (path[0] == '/') {
            printf("%s\n", path + 1);
		} else {
            printf("%s\n", path);
		}
	}
	// 第一次读文件 如果 文件大小不行 或者 文件没有名 那么就直接返回
	if ((r = readn(fd, &fnext, sizeof f)) != sizeof f || fnext.f_name[0] == '\0') {
		return;
	}

	while (nowEnd != 1) {
		f = fnext;
		// 读下一个文件，如果文件大小不行 或者 文件没有名 那么这次完事就结束了
		if ((r = readn(fd, &fnext, sizeof f)) != sizeof f || fnext.f_name[0] == '\0') {
			nowEnd = 1;
		}
		if (f.f_name[0] != '\0') {
			print(deep, isEnd);
			if (nowEnd) {
                printf("`--");
            } else {
                printf("|--");
            }
			printf("%s\n", f.f_name);
		}
		if (f.f_type == FTYPE_DIR) {
			isEnd[deep] = nowEnd;
			strcpy(next_path, path);
			strcpy(next_path + strlen(next_path), "/");
			strcpy(next_path + strlen(next_path), f.f_name);
			tree(next_path, deep + 1, isEnd);
		}
	}
	close(fd);
}

void print(int deep, int* isEnd) {
	int i;
	int tot = deep * 3;
	for (i = 0; i < tot; i++) {
		if (i % 3 == 0 && isEnd[i / 3] != 1) {
            printf("|");
		} else {
			printf(" ");
		}
	}
}

void main(int argc, char **argv) {
	char path[MAXPATHLEN];
	int isEnd[32] = {0};
	
	char t[2] = "/";
	int r = syscall_read_curdir(path, 128);
	memcpy(path + strlen(path), t, 1);
	memcpy(path + strlen(path), argv[1], strlen(argv[1]));

	int fd = open(path, O_RDONLY);
	if (fd < 0) {
        debugf("open %s failed\n", path);
		return;
	}
	struct Filefd* ffd = (struct Filefd *)num2fd(fd);
	if (ffd->f_file.f_type != FTYPE_DIR) {
		debugf("%s is not a dir\n", path);
		return;	
	}
	tree(path, 0, isEnd);
}