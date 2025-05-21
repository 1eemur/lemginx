/* logging.h - Logging functionality */
#ifndef LOGGING_H
#define LOGGING_H

#include "common.h"

void write_log(const char *log_message);
void handle_log(const char *log_message, log_level_t log_level);

extern pthread_mutex_t log_file_mutex;
extern char log_message[MAX_LOG_LENGTH];
extern size_t message_len;

#endif /* LOGGING_H */