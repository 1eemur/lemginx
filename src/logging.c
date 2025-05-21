/* logging.c - Logging functionality implementation */
#include "logging.h"

pthread_mutex_t log_file_mutex = PTHREAD_MUTEX_INITIALIZER; 
char log_message[MAX_LOG_LENGTH];
size_t message_len;

void write_log(const char *log_message) {
    // Check if the log_message is valid
    if (log_message == NULL) {
        fprintf(stderr, "Error: Null log message passed\n");
        return;
    }

    // Open the log file in append mode
    FILE *log_file = fopen(LOG_FILENAME, "a");
    if (log_file == NULL) {
        // Handle file opening failure
        perror("Error opening log file");
        return;
    }

    // Lock the mutex to ensure thread-safe writing to the log file
    pthread_mutex_lock(&log_file_mutex);

    // Write the log message to the file
    if (fputs(log_message, log_file) == EOF) {
        perror("Error writing to log file");
    }

    // Close the file after writing
    if (fclose(log_file) == EOF) {
        perror("Error closing log file");
    }

    // Unlock the mutex
    pthread_mutex_unlock(&log_file_mutex);
}

void handle_log(const char *log_message, log_level_t log_level) {
    time_t now;
    struct tm *timeinfo;
    char timestamp[26];
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
    if (log_message == NULL) return; 
    size_t len = strlen(log_message);
    if (len > MAX_LOG_LENGTH) { 
        fprintf(stderr, "Warning: Log message exceeds maximum length\n");
    }

    // Map log level to a string
    const char *log_level_str;
    switch (log_level) {
        case DEBUG:
            log_level_str = "DEBUG";
            break;
        case INFO:
            log_level_str = "INFO";
            break;
        case WARNING:
            log_level_str = "WARNING";
            break;
        case ERROR:
            log_level_str = "ERROR";
            break;
        case CRITICAL:
            log_level_str = "CRITICAL";
            break;
        default:
            log_level_str = "UNKNOWN";
            break;
    }

    char formatted_log[MAX_LOG_LENGTH];
    size_t msg_len;
    msg_len = snprintf(formatted_log, sizeof(formatted_log), 
                      "[%s][%s]: %s\n", timestamp, log_level_str, log_message);
    if(LOGGING == 1){
        write_log(formatted_log);
    } 
    printf("%s", formatted_log);
}