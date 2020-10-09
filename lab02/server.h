#ifndef LAB02_SOCKET_H_
#define LAB02_SOCKET_H_

#define SERVER_ADDR_PORT 8888
#define BUF_SIZE 256

/* defined in linux/kernel.h */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

int tprintf(const char *format, ...);

#endif  // LAB02_SOCKET_H_
