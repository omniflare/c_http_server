# Simple HTTP Server in C

A lightweight HTTP server implemented in C that serves static files (HTML and images) with proper MIME type detection.

## Features

- Serves static HTML files from `/app` directory
- Serves images from `/img` directory
- Supports multiple file types (HTML, PNG, JPG, GIF)
- Proper MIME type detection
- Basic error handling
- Fork-based request handling for concurrent connections

## Prerequisites

- GCC compiler
- Make build system
- UNIX-like operating system (Linux, macOS)

## Installation

1. Clone the repository:
```bash
git clone <repository-url>
cd http-server
```

2. Build the server:
```bash
make
```

This will:
- Create necessary directories (`app` and `img`)
- Compile the server

## Usage

1. Start the server:
```bash
./httpd <port>
```
Example:
```bash
./httpd 8080
```

2. Directory Structure:
```
.
├── app/          # Place HTML files here
│   └── *.html
├── img/          # Place images here
│   └── *.png, *.jpg, *.gif
└── httpserver    # Server executable
```

3. Accessing Files:
- HTML files: `http://localhost:8080/app/filename.html`
- Images: `http://localhost:8080/img/image.png`

## Supported File Types

- HTML files (`.html`, `.htm`)
- Images:
  - PNG (`.png`)
  - JPEG (`.jpg`, `.jpeg`)
  - GIF (`.gif`)

## Development

### Available Make Commands

- `make`: Build the server
- `make clean`: Clean build files
- `make run`: Run server on port 8080
- `make test`: Run basic server test
- `make install`: Install to system (requires sudo)

### Adding Files

1. HTML Files:
```bash
cp your_file.html app/
```

2. Images:
```bash
cp your_image.png img/
```

Make sure files have proper permissions:
```bash
chmod 644 app/* img/*
```

## Error Handling

The server provides basic error responses:
- 404 for files not found
- 200 for successful requests
- Basic error messages for server issues

## Security Notes

- Server runs on localhost (127.0.0.1) by default
- No directory traversal protection implemented yet
- Limited to static file serving
- No SSL/TLS support

## Limitations

- Maximum URL length: 128 bytes
- Maximum request method length: 8 bytes
- Buffer size for file reading: 512 bytes
- No support for query parameters
- No caching implementation

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## Author

github.com/omniflare

## Acknowledgments

- Built with standard C libraries
- Inspired by basic HTTP protocol specifications
- Inspired by Foundations of Network Technology By Dr. Jonas Birch