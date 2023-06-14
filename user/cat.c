#include <lib.h>

char buf[8192];

void cat(int f, char *s) {
	long n;
	int r;

	while ((n = read(f, buf, (long)sizeof buf)) > 0) {
		if ((r = write(1, buf, n)) != n) {
			user_panic("write error copying %s: %d", s, r);
		}
	}
	if (n < 0) {
		user_panic("error reading %s: %d", s, n);
	}
}

int main(int argc, char **argv) {
	int f, i;

	if (argc == 1) {
		cat(0, "<stdin>");
	} else {
		for (i = 1; i < argc; i++) {
			char tmp[128];
        	char t[2] = "/";
        	int r = syscall_read_curdir(tmp, 128);
     	   	memcpy(tmp + strlen(tmp), t, 1);
    	    memcpy(tmp + strlen(tmp), argv[i], strlen(argv[i]));
    		//debugf("touch: %s\n", tmp);
			f = open(tmp, O_RDONLY);
			if (f < 0) {
				user_panic("can't open %s: %d", tmp, f);
			} else {
				cat(f, tmp);
				close(f);
			}
		}
	}
	return 0;
}
