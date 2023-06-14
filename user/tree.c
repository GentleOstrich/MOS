#include <lib.h>

int tree(char *path, int deep);

int tree(char *path, int deep) {
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
			// 子目录前得有空格来保证格式
			for (int i = 0; i < deep; i++) {
				printf(" ");
			}
			if (nowEnd) {
                printf("`--");
            } else {
                printf("|--");
            }
			printf("%s\n", f.f_name);
		}
		if (f.f_type == FTYPE_DIR) {
			strcpy(next_path, path);
			strcpy(next_path + strlen(next_path), "/");
			strcpy(next_path + strlen(next_path), f.f_name);
			// 对子目录再tree
			tree(next_path, deep + 1);
		}
	}
	close(fd);
}

void main(int argc, char **argv) {
	char path[MAXPATHLEN];
	
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
	tree(path, 0);
}