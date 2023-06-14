#include <lib.h>

void main(int argc, char** argv){
	int r;
	if (argc < 2) {
        debugf("too few argv\n");
		return;
	} else if (argc > 2) {
        debugf("too many argv\n");
		return;
    } else {
        char path[MAXPATHLEN];
        char t[2] = "/";
        int r = syscall_read_curdir(path, 128);
        memcpy(path + strlen(path), t, 1);
        memcpy(path + strlen(path), argv[1], strlen(argv[1]));
        r = create(path, 0);
    }
    if (r < 0) {
        debugf("cannot create dir\n");
    }
}