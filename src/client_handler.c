/* client_handler.c - Client request handling logic implementation */
#include "client_handler.h"
#include "file_utils.h"
#include "http_utils.h"
#include "logging.h"

// Function to handle client request data, making sure information is properly extracted and decoded
void *handle_client(void *arg) {
    int client_fd = *((int *)arg);
    char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

    // Get the server directory
    char *server_dir = get_server_directory();
    if(server_dir == NULL){
        close(client_fd);
        free(arg);
        free(buffer);
        return NULL;
    }

    // Get client address for logging
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_fd, (struct sockaddr *)&addr, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    
    // Take the request from the client and store into a buffer
    ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
    if(bytes_received > 0){
        // Check request type 
        regex_t regex;
        regcomp(&regex, "^([A-Z]+) /([^ ]*) HTTP/1", REG_EXTENDED);
        regmatch_t matches[3];

        if(regexec(&regex, buffer, 3, matches, 0) == 0){
            // Extract filename from request and decode URL
            buffer[matches[1].rm_eo] = '\0';
            buffer[matches[2].rm_eo] = '\0';
            const char *method = buffer + matches[1].rm_so;
            const char *url_encoded_file_name = buffer + matches[2].rm_so;

            // If no path is provided (i.e., the path is empty), use "index.html" as the default
            char *file_name;
            if(strlen(url_encoded_file_name) == 0){
                file_name = strdup("index.html");
                message_len = snprintf(log_message, sizeof(log_message), 
                                    "[INFO] File not provided, serving index.html as default");
                handle_log(log_message, INFO);
                } else {
                    file_name = url_decode(url_encoded_file_name);
                }

            // Sanitize file path
            char *safe_file_name = sanitize_file_path(file_name);
            if (safe_file_name == NULL) {
                    // Invalid file path, respond with 400 Bad Request
                    char *response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n400 Bad Request";
                    send(client_fd, response, strlen(response), 0);
                    free(file_name);
                    free(server_dir);
                    close(client_fd);
                    free(arg);
                    free(buffer);
                    return NULL;
            }

            // Build the full file path
            char full_file_path[PATH_MAX];
            snprintf(full_file_path, PATH_MAX, "%s/%s", server_dir, safe_file_name);

            message_len = snprintf(log_message, sizeof(log_message), 
                                  "%s request from %s for /%s", method, client_ip, file_name);
            handle_log(log_message, INFO);
            message_len = snprintf(log_message, sizeof(log_message), 
                                  "Full file path %s", full_file_path);
            handle_log(log_message, INFO);

            // Get the file extension
            char file_ext[32];
            strcpy(file_ext, get_file_extension(file_name));

            // Build the HTTP response
            char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
            size_t response_len;

            // Check if file exists
            char *actual_file = get_file_case_insensitive(full_file_path);
            if(actual_file != NULL) {
                    char file_ext[32];
                    strcpy(file_ext, get_file_extension(actual_file));

                    // Build the HTTP response
                    build_http_response(actual_file, file_ext, response, &response_len);
                    send(client_fd, response, response_len, 0);
                    free(response);
                    free(safe_file_name);
            } else {
                    // File not found
                    snprintf(buffer, BUFFER_SIZE,
                            "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found");
                    send(client_fd, buffer, strlen(buffer), 0);
                    free(safe_file_name);
                    message_len = snprintf(log_message, sizeof(log_message), 
                                          "File not found: %s — sending 404 response", 
                                          file_name);
                    handle_log(log_message, ERROR);
            }
            free(file_name);
        }
        regfree(&regex);
    }
    // Close connection and free any allocated memory before returning NULL
    free(server_dir);
    close(client_fd);
    free(arg);
    free(buffer);
    return NULL;
}