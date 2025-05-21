/* common.h - Common definitions and includes */
#ifndef COMMON_H
#define COMMON_H

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 104857600
#define LOGGING 0
#define MAX_LOG_LENGTH 1024
#define LOG_FILENAME "lemginx_log.txt"

typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
} log_level_t;

#endif /* COMMON_H */