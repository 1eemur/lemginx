# lemginx - A Lightweight HTTP Server

Super simple web server written in C for learning purposes. Created to get a better understanding of web sockets.

## Features

- **MIME type support**: Correctly serves different file types with appropriate Content-Type headers
- **URL decoding**: Properly handles encoded URLs
- **Path sanitization**: Prevents directory traversal attacks
- **Logging system**: Configurable logging with different severity levels

## Project Structure

The project is organized into several modules, each with a specific responsibility:

- **main.c**: Entry point that sets up the server socket and accepts connections
- **common.h**: Common definitions, constants, and includes shared across the project
- **logging.h/c**: Handles all logging functionality with thread-safe operations
- **file_utils.h/c**: Manages file operations (paths, URL decoding, etc.)
- **http_utils.h/c**: Contains HTTP-specific utilities (MIME types, response building)
- **client_handler.h/c**: Processes client connections and HTTP requests
- **Makefile**: Automates the build process

## Getting Started

### Building the Project

1. Clone the repository:
   ```bash
   git clone repo
   cd lemginx
   ```

2. Build the project using make:
   ```bash
   make
   ```

3. Run the server:
   ```bash
   ./lemginx
   ```

The server will start listening on port 8080 by default.

### Usage

Once the server is running, you can access it using a web browser or tools like curl:

```bash
curl http://localhost:8080/
```

By default, the server will look for an `index.html` file in the current directory.

## Configuration

You can modify the following settings in `common.h`:

- `PORT`: The port number the server listens on (default: 8080)
- `BUFFER_SIZE`: Maximum buffer size for requests and responses
- `LOGGING`: Enable (1) or disable (0) file logging
- `LOG_FILENAME`: Name of the log file
- `MAX_LOG_LENGTH`: Maximum length of a log message

## To-Do
- `public_html`: Implement better path management for where web content is stored
- `lemgnix.conf`: Create a config file allow customization for some of the variables currently set in common.h
- `404.html`: Add default pages for common errors like 404
