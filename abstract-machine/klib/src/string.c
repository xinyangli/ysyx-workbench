#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *p = s;
  size_t len = 0;
  while(*(p++) != '\0') len++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *p_dst = dst;
  const char *p_src = src;
  for(; *p_src != '\0'; p_src++, p_dst++) {
    *p_dst = *p_src;
  }
  *p_dst = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i = 0;
  for(; i < n && src[i] != '\0'; i++) {
    dst[i] = src[i];
  }
  for(; i < n; i++) {
    dst[i] = '\0';
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *p_dst = dst;
  const char *p_src = src;
  while(*p_dst != '\0') p_dst++;
  for(; *p_src != '\0'; p_src++, p_dst++) {
    *p_dst = *p_src;
  }
  *p_dst = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const char *p_s1 = s1, *p_s2 = s2;
  for(; *p_s1 == *p_s2; p_s1++, p_s2++) {
    if(*p_s1 == '\0' || *p_s2 == '\0') {
      break;
    }
  } 
  return *p_s1 - *p_s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const char *p_s1 = s1, *p_s2 = s2;
  int i = 0;
  for(i = 0; i < n - 1; i++) {
    if(s1[i] == '\0' || s2[i] == '\0')
      break;
  } 
  return s1[i] - s2[i];
}

void *memset(void *s, int c, size_t n) {
  uint8_t *p = s;
  for(int i = 0; i < n; i++) {
    p[i] = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  if (src + n  > dst && src < dst) {
    size_t len = dst - src;
    void *p_dst = (void *)src + n;
    const void *p_src = src + n - len;
    while(p_dst >= dst) {
      memcpy(p_dst, p_src, len);
      p_src -= len;
      p_dst -= len;
    }
    if(n % len) memcpy(dst, src, n % len);
  } else if (dst < src && dst + n > src) {
    size_t len = src - dst;
    void *p_dst = dst;
    const void *p_src = src;
    while(p_src < src + n) {
      memcpy(p_dst, p_src, len);
      p_src += len;
      p_dst += len;
    }
    if(n % len) memcpy(p_dst, p_src, n % len);
  } else { 
    memcpy(dst, src, n);
  }

  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  for (size_t i = 0 ; i < n ; i++) {
    *(uint8_t *)(out + i) = *(uint8_t *)(in + i);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const uint8_t *p1 = s1, *p2 = s2;
  for (int i = 0; i < n; i++) {
    if(*p1 != *p2)
      return p1 - p2;
    p1++; p2++;
  }
  return 0;
}

#endif
