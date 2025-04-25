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

#define PORT 8080
#define BUFFER_SIZE 104857600

char *get_server_directory() {
	char full_path[PATH_MAX];
	// Get the full path of the current source file (main.c)
	if (realpath(__FILE__, full_path) == NULL) {
			perror("realpath error");
			return NULL;
	}
	// Get the directory of the source file (dirname)
	char *dir = dirname(full_path);
	// Return a copy of the directory path
	char *server_dir = strdup(dir);
	if (server_dir == NULL) {
			perror("strdup error");
			return NULL;
	}
	return server_dir;
}

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

bool case_insensitive_compare(const char *s1, const char *s2) {
	// Iterate through strings to compare characters
	while(*s1 && *s2){
		if(tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return false;
		s1++;
		s2++;
	}
	return *s1 == *s2;
}
#include <libgen.h>  // For basename

char *get_file_case_insensitive(const char *file_name) {
    // Extract the base name (file name) from the full path
    char *base_name = basename(file_name);  // Extracts the filename from the path

    // Open current directory
    DIR *dir = opendir(".");
    // Error to check to confirm that the directory has been successfully opened
    if (dir == NULL) {
        perror("opendir error");
        return NULL;
    }

    // Create directory stream and check directory for provided file name
    struct dirent *entry;
    char *found_file_name = NULL;
    while ((entry = readdir(dir)) != NULL) {
        if (case_insensitive_compare(entry->d_name, base_name)) {
            found_file_name = entry->d_name;
            break;
        }
    }

    // Close directory and return found file
    closedir(dir);
    return found_file_name;
}

char *sanitize_file_path(const char *file_name) {
	// Prevent directory traversal by disallowing '..' or leading '/'
	// Proper input sanitization to be added later
	if(strstr(file_name, "../") != NULL || file_name[0] == '/') {
			fprintf(stderr, "[ERROR] Invalid file path: %s\n", file_name);
			return NULL;
	}

	return strdup(file_name);
}

char *url_decode(const char *src) {
	size_t src_len = strlen(src);
	char *decoded = malloc(src_len + 1);
	size_t decoded_len = 0;

	// Decode to hex
	for(size_t i = 0; i < src_len; i++){
		if(src[i] == '%' && i + 2 < src_len){
			int hex_val;
			sscanf(src + i + 1, "%2x", &hex_val);
			decoded[decoded_len++] = hex_val;
			i += 2;
		}
		else decoded[decoded_len++] = src[i];
	}
	
	// Ensure that string is null-terminated
	decoded[decoded_len] = '\0';
	return decoded;
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
		fprintf(stderr, "[ERROR] File not found: %s — sending 404 response\n", file_name);
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
	printf("[INFO] Serving file: %s (MIME type: %s)\n", file_name, mime_type);

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

	// Log how many bytes were sent
	printf("[INFO] Sent %zu bytes in response for %s\n", *response_len, file_name);

	// Free header memory and close file
	free(header);
	close(file_fd);
}

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
				printf("[INFO] File not provided, serving index.html as default\n");
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
					free(safe_file_name);
					free(server_dir);
					close(client_fd);
					free(arg);
					free(buffer);
					return NULL;
			}

			// Build the full file path
			char full_file_path[PATH_MAX];
			snprintf(full_file_path, PATH_MAX, "%s/%s", server_dir, safe_file_name);

			printf("[INFO] %s request from %s for /%s\n", method, client_ip, file_name);
			printf("[INFO] Full file path %s\n", full_file_path);

			// Get the file extension
			char file_ext[32];
			strcpy(file_ext, get_file_extension(file_name));

			// Build the HTTP response
			char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
			size_t response_len;

			int file_fd = open(full_file_path, O_RDONLY);
			// Check if file exists
			char *actual_file = get_file_case_insensitive(full_file_path);
			if(actual_file != NULL) {
					char file_ext[32];
					strcpy(file_ext, get_file_extension(actual_file));

					// Build the HTTP response
					char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
					size_t response_len;
					build_http_response(actual_file, file_ext, response, &response_len);
					send(client_fd, response, response_len, 0);
					free(response);
					free(safe_file_name);
			}else {
					// File not found
					snprintf(buffer, BUFFER_SIZE,
										"HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found");
					send(client_fd, buffer, strlen(buffer), 0);
					free(safe_file_name);
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

int main() {
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);
	int server_fd, max_connections = 10;
	struct sockaddr_in server_addr;

	// Create the server socket
	printf("lemgnix starting\n");
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("Socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Configure the server socket
	// AF_INET, use IPv4 (AF_INET6 for IPv6)
	server_addr.sin_family = AF_INET;
	// INADDR_ANY, tells the server to accept connections from any network interface
	server_addr.sin_addr.s_addr = INADDR_ANY;
	// Convert hostbyte order from Least Significant Bit first to network byte order, which is Most Signficiant byte first
	server_addr.sin_port = htons(PORT);

	// Binds the socket to a port defined above, causing the socket to listen for any clients trying connect on that port
	printf("Binding socket to port %d\n", PORT);
	if(bind(server_fd, 
			(struct sockaddr*)&server_addr,
			sizeof(server_addr)) < 0){
		perror("Bind failed");
		exit(EXIT_FAILURE);
	}

	// Listen for connections
	printf("Listening for connections on port %d\n", PORT);
	if(listen(server_fd, max_connections) < 0){
		perror("Listen failed");
		exit(EXIT_FAILURE);
	}

	while(1){
		// Define client info
		struct sockaddr_in client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		int *client_fd = malloc(sizeof(int));

		// Accpt client connections
		if((*client_fd = accept(server_fd,
						(struct sockaddr *)&client_addr, 
						&client_addr_len)) < 0){
			perror("Client accept failed");
			continue;
		}

		// Create new thread to handle client request
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
		pthread_detach(thread_id);
	}
	return 0;
}
