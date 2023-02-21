#include <blib.h>

size_t strlen(const char *s) {
	size_t i = 0;
	while (*s++) {
		i++;	
	}
	return i;
}

char *strcpy(char *dst, const char *src) {
	char *res = dst;
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strncpy(char *dst, const char *src, size_t n) {
	char *res = dst;
	while (*src && n--) {
		*dst++ = *src++;
	}
	*dst = '\0';
	return res;
}

char *strcat(char *dst, const char *src) {
	char *ptr = dst + strlen(dst);
	while (*src != '\0') {
		*ptr++ = *src++;
	}
	*ptr = '\0';
	return dst;

}

int strcmp(const char *s1, const char *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2) return *s1 - *s2;
	
		s1++, s2++;
	}
	return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	while (n--) {
		if (*s1 != *s2) {
			return *s1 - *s2;
		}
		if (*s1 == 0) {
			break;
		}
		s1++;
		s2++;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
	char *p = (char *)s;
	while (n--) {
		*p++ = c;
	}
	return s;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *csrc = (char *)in;
	char *cdest = (char *)out;
	for (int i = 0; i < n; i++) {
		cdest[i] = csrc[i];
	}
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	char *cs1 = (char *)s1, *cs2 = (char *)s2;
	while (n--) {
		if (*cs1 != *cs2) return *cs1 - *cs2;
		if (*cs1 == 0) break;
		cs1++, cs2++;
	}
	return 0;
}
