#include <lib.h>

void main(int argc, char** argv) {
    int fd = open("/.history", O_RDONLY);
	char buf[128];
	int r;
	if (fd < 0) {
		debugf("\n./history not found\n");
	}
	int cnt = 1;
	while ((r = read(fd, buf, 128)) != 0 ) {
		printf("%s\n", buf);
	}
}