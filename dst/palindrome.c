#include <stdio.h>
#include <string.h>
int func(int);
int main() {
	int n;
	scanf("%d", &n);

	if (func(n)) {
		printf("Y\n");
	} else {
		printf("N\n");
	}
	return 0;
}

int func(int x) {
	int c[100];
	int i = 0;
	while (x) {
		c[i++] = x % 10;
		x /= 10;
	}
	int j = i - 1;
	i = 0;
	for (;i <= j; ++i, --j) {
		if (c[i] != c[j]) {
			return 0;
		}
	}
	return 1;
}
