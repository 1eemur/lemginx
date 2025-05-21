/* http_utils.c - HTTP response building and MIME type handling implementation */
#include "http_utils.h"
#include "logging.h"

const char *get_file_extension(const char *file_name) {
    // Returns a pointer to the last dot in the string provided
    const char *dot = strrchr(file_name, '.');
    if(!dot || dot == file_name) return "";
    return dot + 1;
}

const char *get_mime_type(const char *file_ext) {
    // Compare file extension with common file extensions served by a web server
    if(strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) return "text/html";
    else if(strcasecmp(file_ext, "txt") == 0) return "text/plain";
    else if(strcasecmp(file_ext, "jpg") == 0 || strcasecmp(file_ext, "jpeg") == 0) return "image/jpeg";
    else if(strcasecmp(file_ext, "png") == 0) return "image/png";
    else return "application/octet-stream";
}

void build_http_response(const char *file_name, 
                        const char *file_ext, 
                        char *response, 
                        size_t *response_len) {
    // Build HTTP header for response
    const char *mime_type = get_mime_type(file_ext);
    char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(header, BUFFER_SIZE,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "\r\n",
        mime_type);

    // Check if the file exists, if not return 404 as the HTTP response
    int file_fd = open(file_name, O_RDONLY);
    if(file_fd == -1){
        printf("[ERROR] File not found: %s — sending 404 response", file_name);
        snprintf(response, BUFFER_SIZE,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "\r\n"
            "404 Not Found");
        *response_len = strlen(response);
        free(header);
        return;
    }

    // Log file being served
    message_len = snprintf(log_message, sizeof(log_message), 
                          "[INFO] Serving file: %s (MIME type: %s)", 
                          file_name, mime_type);
    handle_log(log_message, INFO);

    // Get file size for Content-Length
    struct stat file_stat;
    if(fstat(file_fd, &file_stat) == -1){
        perror("[ERROR] Could not get file stats");
    }
    off_t file_size = file_stat.st_size;

    // Copy header to the response buffer
    *response_len = 0;
    memcpy(response, header, strlen(header));
    *response_len += strlen(header);

    // Copy file to the response buffer
    ssize_t bytes_read;
    while((bytes_read = read(file_fd, 
                            response + *response_len, 
                            BUFFER_SIZE - *response_len)) > 0){
        *response_len += bytes_read;
    }

    message_len = snprintf(log_message, sizeof(log_message), 
                          "Sent %zu bytes in response for %s", 
                          *response_len, file_name);
    handle_log(log_message, INFO);

    // Free header memory and close file
    free(header);
    close(file_fd);
}