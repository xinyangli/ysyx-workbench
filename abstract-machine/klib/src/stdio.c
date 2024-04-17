#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void print_int(int num, int width, char pad) {
  int reverse = 0;
  int count = 0;

  if (num == 0) {
    reverse = 0;
    count = 1;
  } else {
    if (num < 0) {
      putch('-');
      num = -num;
    }
    while (num != 0) {
      reverse = reverse * 10 + (num % 10);
      num /= 10;
      count++;
    }
  }

  while (width > count) {
    putch(pad);
    width--;
  }

  if (reverse == 0) {
    putch('0');
  } else {
    while (reverse != 0) {
      putch('0' + (reverse % 10));
      reverse /= 10;
    }
  }
}

int vprintf(const char *format, va_list args) {
  const char *p = format;

  while (*p) {
    if (*p == '%') {
      p++; // Skip the '%'
      char pad = ' ';
      int width = 0;

      // Handle zero padding
      if (*p == '0') {
        pad = '0';
        p++;
      }

      // Handle width
      while (*p >= '0' && *p <= '9') {
        width = width * 10 + (*p - '0');
        p++;
      }

      switch (*p) {
      case 'd': { // Integer
        int ival = va_arg(args, int);
        print_int(ival, width, pad);
        break;
      }
      case 'c': { // Character
        char c = (char)va_arg(args, int);
        putch(c);
        break;
      }
      case 's': { // String
        char *s = va_arg(args, char *);
        putstr(s);
        break;
      }
      case '%': {
        putch('%');
        break;
      }
      default:
        panic_on(true, "Wrong formatter provided to printf");
      }
    } else {
      putch(*p);
    }
    p++;
  }
}

int printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  return 0;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
