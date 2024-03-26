#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

void test_strcpy() {
    char b[32];
    char *s;
 	b[16]='a'; b[17]='b'; b[18]='c'; b[19]=0;
 	panic_on((s = strcpy(b, b+16)) != b, "strcpy wrong return value");
	panic_on(strcmp(s, "abc") != 0, "strcpy gave incorrect string");
 	panic_on((s = strcpy(b+1, b+16)) != b+1, "strcpy wrong return value");
	panic_on(strcmp(s, "abc") != 0, "strcpy gave incorrect string");

	panic_on((s = strcpy(b+1, b+17)) != b+1, "strcpy wrong return value");
	panic_on(strcmp(s, "bc") != 0, "strcpy gave incorrect string");
}

void test_strncpy() {
    char b[32];
	char *s;
	int i;
    b[3] = 'x'; b[4] = 0;
	panic_on((s = strncpy(b, "abc", 3)) != b, "strncpy wrong return value");
    panic_on(b[2] != 'c', "strncpy fails to copy last byte");
    panic_on(b[3] != 'x', "strncpy overruns buffer to null-terminate");
}

void test_strncmp() {
	panic_on(strncmp("abcd", "abce", 3) != 0, "strncmp compares past n");
	panic_on(strncmp("abc", "abd", 3) == 0, "strncmp fails to compare n-1st byte");
}

void test_memset() {
    uint8_t arr[128];
    arr[120] = 0xd;
    panic_on(memset(arr, 0xf, 120) != arr, "memset wrong return value");
    panic_on(arr[7] != 0xf, "memset fails to set value in range");
    panic_on(arr[120] != 0xd, "memset set value past n");
}

void test_memcpy() {
    const uint8_t src[] = { 0x0, 0x0, 0x1, 0x2, 0x3, 0x4, 0x0, 0x0 };
    uint8_t dst[8] = {0};
    memcpy(dst, src, 8);
    panic_on(memcmp(dst, src, 8) != 0, "memcpy fails to copy memory");
}

void test_memmove() {
    const uint8_t ref[] = { 0x0, 0x0, 0x1, 0x2, 0x3, 0x4, 0x0, 0x0 };
    uint8_t dst[8] = {0};
    const uint8_t ans1[] = { 0x1, 0x2, 0x3, 0x4, 0x3, 0x4, 0x0, 0x0 };
    const uint8_t ans2[] = { 0x1, 0x2, 0x2, 0x3, 0x4, 0x3, 0x0, 0x0 };
    const uint8_t ans3[] = { 0x1, 0x2, 0x2, 0x1, 0x2, 0x2, 0x3, 0x4 };
    memmove(dst, ref, 8);
    panic_on(memcmp(dst, ref, 8) != 0, "memmove fails to copy non-overlapping memory");

    memmove(dst, dst + 2, 4);
    panic_on(memcmp(dst, ans1, 8) != 0, "memmove fails to copy overlapping memory (dst < src)");

    memmove(dst + 2, dst + 1, 4);
    panic_on(memcmp(dst, ans2, 8) != 0, "memmove fails to copy overlapping memory (src < dst)");

    memmove(dst + 3, dst, 5);
    panic_on(memcmp(dst, ans3, 8) != 0, "memmove fails to copy overlapping memory (src < dst)");
}

int main(void) {
    test_strcpy();
    test_strncpy();
    test_strncmp();
    test_memset();
    test_memcpy();
    test_memmove();
    return 0;
}
