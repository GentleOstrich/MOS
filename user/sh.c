#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"
static int doBuiltInCmd(int argc, char**argv); 

static int doBuiltInCmd(int argc, char**argv) {
	if (strcmp(argv[0],"cd") == 0) {
		if (argc != 2) {
			user_panic("cd usage : cd path\n");
		}
		chdir(argv[1]);
		return 1;
	} else if (strcmp(argv[0],"pwd") == 0) {
		char buf[128];
		if (argc != 1) {
			user_panic("pwd usage : pwd\n");
		}
		getcwd(buf,sizeof(buf));
		printf("%s\n",buf);	
		return 1;
	}
	return 0;
}

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	int flag = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}
	if (*s == '\"') {
		flag = 1;
		strcpy(s, s + 1);
	}
	if (strchr(SYMBOLS, *s) && flag == 0) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}

	*p1 = s;
	while (*s && (!strchr(WHITESPACE SYMBOLS, *s) || flag == 1)) {
		if (*s == '\"') {
			flag = 0;
			strcpy(s, s + 1);
		} else {
			s++;
		}
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe, int *running) {
	int argc = 0;
	while (1) {
		char *t;
		int fd, r;
		int c = gettoken(0, &t);
		switch (c) {
		case ';':
			*running = fork();
			if (*running == 0) {
				return parsecmd(argv, rightpipe, running);
			} else {
				return argc;
			}
			break;
		case '&':
			argv[argc] = 0;
			int child = spawn(argv[0], argv);
			argc = 0;
			break;
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			// Open 't' for reading, dup it onto fd 0, and then close the original fd.
			/* Exercise 6.5: Your code here. (1/3) */
			fd = open(t, O_RDONLY);
			dup(fd, 0);
			close(fd);
			//user_panic("< redirection not implemented");

			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			// Open 't' for writing, dup it onto fd 1, and then close the original fd.
			/* Exercise 6.5: Your code here. (2/3) */
			fd = open(t, O_WRONLY);
			dup(fd, 1);
			close(fd);
			//user_panic("> redirection not implemented");

			break;
		case '|':;
			/*
			 * First, allocate a pipe.
			 * Then fork, set '*rightpipe' to the returned child envid or zero.
			 * The child runs the right side of the pipe:
			 * - dup the read end of the pipe onto 0
			 * - close the read end of the pipe
			 * - close the write end of the pipe
			 * - and 'return parsecmd(argv, rightpipe)' again, to parse the rest of the
			 *   command line.
			 * The parent runs the left side of the pipe:
			 * - dup the write end of the pipe onto 1
			 * - close the write end of the pipe
			 * - close the read end of the pipe
			 * - and 'return argc', to execute the left of the pipeline.
			 */
			int p[2];
			/* Exercise 6.5: Your code here. (3/3) */
			pipe(p);
			*rightpipe = fork();
			if (*rightpipe == 0) {
				dup(p[0], 0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv, rightpipe, running);
			}  else if (*rightpipe > 0) {
				dup(p[1], 1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			//user_panic("| not implemented");
			break;
		}
	}

	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);
	int r;
	char *argv[MAXARGS];
	int rightpipe = 0;
	int running;
	int argc = parsecmd(argv, &rightpipe, &running);
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;
	if (doBuiltInCmd(argc,argv) != 0) {
		return;
	} 
	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0) {
		wait(child);
	} else {
		debugf("spawn %s: %d\n", argv[0], child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	if (running) {
		wait(running);
	}
	exit();
}

int history_cnt = 0;

int read_history(char history_cmd[128][128]){
	int index = ((history_cnt - 128) > 0) ? history_cnt - 128 : 0;
	int i;
	char buf[128];
	int fd = open("/.history", O_RDONLY);
	if(fd < 0){
		return 0;
	}
	for(i = 0; i< index; i++){
		debugf("\njump index\n");
		read_line(fd, buf, 128);
	}
	for (i=index; i < history_cnt; i++) {
		read_line(fd, buf, 128);
		strcpy(history_cmd[i-index], buf);
	}
	close(fd);
	return history_cnt - index;
}

void write_history(char* buf, int n) {
	static int run_time = 0;
	int fd, r;
	if(run_time == 0){
		create("/.history", 0);
	}
	fd = open("/.history", O_RDWR | O_APPEND);
	r = write(fd, buf, n);
	if (r < n) {
		debugf("\nerror in write_history\n");
	}
	r = write(fd, "\n", 1);
	if (r < 1) {
		debugf("\nerror in write_history\n ");
	}
	close(fd);
	history_cnt += 1;
	run_time++;
}

void readline(char *buf, u_int n) {
	int r;
	char historys[128][128];
	int cnt = read_history(historys);
	int sum = cnt;
	int num = 0;
	int move = 0; // 往左走了几步
	char ch;
	
	for (int i = 0; i < n; i++) {
		if ((r = read_insert(0, buf, 1, i, i)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		
		num++;
		// 上一条
		if (i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 65) {
			printf("%c%c%c", 27, 91, 66);
			i -= 2;
			for (i; i; --i) {
				printf("\b \b");
			}
			if (cnt > 0) {
				strcpy(buf, historys[--cnt]);
			} else {
				strcpy(buf, historys[cnt]);
			}
			printf("%s", buf);
			i = strlen(buf) - 1;
			num = i + 1;
		}
		// 下一条
		if (i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 66) {
			if (cnt < sum - 1) {
				i -= 2;
				for (i; i; --i) {
					printf("\b \b");
				}
				strcpy(buf, historys[++cnt]);
				printf("%s", buf);
				i = strlen(buf) - 1;
			} else if (cnt == sum) {
				buf[i - 2] = buf[i - 1] = buf[i] = 0;
				i = strlen(buf) - 1;
			} else {
				i -= 2;
				for (i; i; --i) {
					printf("\b \b");
				}
				cnt++;
				buf[0] = 0;
				i = -1; 
			}
			num = i + 1;
		}
		// 左移   
		if (i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 68) {
			num -= 3;
			printf("%c%c%c", 27, 91, 67);
			for (int k = 0; k < move; ++k) {
				printf("%c%c%c", 27, 91, 67);
			}
			char c = buf[num];
			buf[num] = 0;
			for (int j = 0; j < strlen(buf); ++j) {
			 	printf("\b \b");
			}  
			buf[num] = c;
			memcpy(buf + i - 2, buf + i + 1, 128);
			buf[num] = 0;
			printf("%s", buf);
			i -= 4;
			if (i < 0) {
				i = -1;
			}
			if (move < num) {
				move++;
			}
			for (int k = 0; k < move; ++k) {
				printf("%c%c%c", 27, 91, 68);
			}
			continue;
		}
		// 右移
		if (i >= 2 && buf[i - 2] == 27 && buf[i - 1] == 91 && buf[i] == 67) {
			printf("%c%c%c", 27, 91, 68);
			if (move > 0) {
				num -= 3;
				for (int k = 0; k < move; ++k) {
					printf("%c%c%c", 27, 91, 67);
				}
				char c = buf[num];
				buf[num] = 0;
				for (int j = 0; j < strlen(buf); ++j) {
					printf("\b \b");
				}
				buf[num] = c;
				memcpy(buf + i - 2, buf + i + 1, 128);
				buf[num] = 0;
				printf("%s", buf);
				i -= 2;
				if (i < 0) {
					i = -1;
				}
				if (move > 0) {
					move--;
				}
				for (int k = 0; k < move; ++k) {
					printf("%c%c%c", 27, 91, 68);
				}
			} else {
				num -= 3;
				i -= 3;
				if (i < 0) {
					i = -1;
				}
			}
			continue;
		}
		if (buf[i] == '\b' || buf[i] == 0x7f) {
			if (i > 0) {
				buf[i] = 0;
				for (int j = 0; j < strlen(buf); ++j) {
					printf("\b \b");
				}
				buf[i - 1] = 0;
				memcpy(buf + i - 1, buf + i + 1, 128);
				i -= 2;
				num -= 2;
				buf[num] = 0;
				printf("%s", buf);
			} else {
				buf[i] = 0;
				i = -1;
				num = 0;
			}
		}
		if (buf[i] == '\r' || buf[i] == '\n') {
			num -= 1;
			memcpy(buf + i, buf + i + 1, 128);
			buf[num] = 0;
			buf[num + 1] = '\n';
			return;
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}

int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		write_history(buf, strlen(buf));
		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
