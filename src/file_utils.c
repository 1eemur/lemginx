/* file_utils.c - File handling utilities implementation */
#include "file_utils.h"
#include "logging.h"

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

bool case_insensitive_compare(const char *s1, const char *s2) {
    // Iterate through strings to compare characters
    while(*s1 && *s2){
        if(tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) return false;
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

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