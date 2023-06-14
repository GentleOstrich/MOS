#include <lib.h>

int getpwd(char *buf, size_t size) {
	return syscall_read_curdir(buf, size);
}


int chdir(const char *path) {
	int r,i;
	struct Stat st;
	if (path[0] == '/') { // 绝对路径
		if ((r = stat(path, &st) < 0)) {
			printf("chdir : can't open\n");			
			return -1;
		}
		if (st.st_isdir == 0) {
			printf("chdir : not dir\n");
			return -1;
		}
		return syscall_write_curdir(path, sizeof(path));
	} else { // 相对路径
		char buf[MAXPATHLEN];
		syscall_read_curdir(buf, sizeof(buf));
		int len = strlen(buf);
		if (buf[len - 1] != '/') {
			buf[len++] = '/';
		}
		for (i = 0; i < path[i] != 0; ++i) {
			buf[len + i] = path[i];
		}		
		buf[len + i] = 0;
		if ((r = stat(buf,&st)) < 0) {
			printf("chdir : can't open\n");
			return -1;
		}
		return syscall_write_curdir(buf,sizeof(buf));
	}
	return 0;
}