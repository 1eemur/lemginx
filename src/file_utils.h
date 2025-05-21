/* file_utils.h - File handling utilities */
#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "common.h"

char *get_server_directory();
bool case_insensitive_compare(const char *s1, const char *s2);
char *get_file_case_insensitive(const char *file_name);
char *sanitize_file_path(const char *file_name);
char *url_decode(const char *src);

#endif /* FILE_UTILS_H */