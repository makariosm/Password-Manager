#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "lab3.h"
#include "http_server.h"

#define PORT 8080
#define BUFFER_SIZE 1024

// Global KV store reference
extern kvstore_t kv;

// Helper function to send HTTP response with CORS headers
void send_response(int client_socket, int status_code, const char* content_type, const char* body) {
    char response[BUFFER_SIZE];
    sprintf(response, 
            "HTTP/1.1 %d OK\r\n"
            "Content-Type: %s\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, PUT, DELETE\r\n"
            "Access-Control-Allow-Headers: Content-Type\r\n"
            "\r\n%s",
            status_code, content_type, body);
    write(client_socket, response, strlen(response));
}

// Parse the request body as JSON
json_object* parse_request_body(char* body) {
    // Find the start of JSON data (after headers)
    char* json_start = strstr(body, "\r\n\r\n");
    if (json_start == NULL) return NULL;
    json_start += 4; // Move past \r\n\r\n

    return json_tokener_parse(json_start);
}

// Handle GET /api/keys - List all keys
void handle_list_keys(int client_socket) {
    json_object* response_obj = json_object_new_object();
    json_object* pairs_array = json_object_new_array();
    
    // Use existing thread-safe functions to access the data
    for (size_t i = 0; i < kv.num_keys; i++) {
        int value;
        if (kv_read(&kv, kv.keys[i].key, &value) == 0) {  // Using thread-safe kv_read
            json_object* pair = json_object_new_object();
            json_object_object_add(pair, "key", json_object_new_string(kv.keys[i].key));
            json_object_object_add(pair, "value", json_object_new_int(value));
            json_object_array_add(pairs_array, pair);
        }
    }
    
    json_object_object_add(response_obj, "pairs", pairs_array);
    const char* json_str = json_object_to_json_string(response_obj);
    send_response(client_socket, 200, "application/json", json_str);
    
    json_object_put(response_obj);
}

// Handle GET /api/keys/{key} - Get specific key
void handle_get_key(int client_socket, const char* key) {
    int value;
    json_object* response_obj = json_object_new_object();
    
    int result = kv_read(&kv, (char*)key, &value);
    
    if (result == 0) {
        json_object_object_add(response_obj, "key", json_object_new_string(key));
        json_object_object_add(response_obj, "value", json_object_new_int(value));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 200, "application/json", json_str);
    } else {
        json_object_object_add(response_obj, "error", json_object_new_string("Key not found"));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 404, "application/json", json_str);
    }
    
    json_object_put(response_obj);
}

// Handle POST /api/keys - Create new key-value pair
void handle_create_key(int client_socket, char* body) {
    json_object* request = parse_request_body(body);
    if (!request) {
        send_response(client_socket, 400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    json_object* key_obj;
    json_object* value_obj;
    if (!json_object_object_get_ex(request, "key", &key_obj) ||
        !json_object_object_get_ex(request, "value", &value_obj)) {
        send_response(client_socket, 400, "application/json", "{\"error\":\"Missing key or value\"}");
        json_object_put(request);
        return;
    }
    
    const char* key = json_object_get_string(key_obj);
    int value = json_object_get_int(value_obj);
    
    int result = kv_write(&kv, (char*)key, value);
    
    json_object* response_obj = json_object_new_object();
    if (result == 0) {
        json_object_object_add(response_obj, "message", json_object_new_string("Key created successfully"));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 201, "application/json", json_str);
    } else {
        json_object_object_add(response_obj, "error", json_object_new_string("Failed to create key"));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 500, "application/json", json_str);
    }
    
    json_object_put(response_obj);
    json_object_put(request);
}

// Handle PUT /api/keys/{key}/increment - Increment value
void handle_increment_key(int client_socket, const char* key, char* body) {
    json_object* request = parse_request_body(body);
    if (!request) {
        send_response(client_socket, 400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    json_object* amount_obj;
    if (!json_object_object_get_ex(request, "amount", &amount_obj)) {
        send_response(client_socket, 400, "application/json", "{\"error\":\"Missing increment amount\"}");
        json_object_put(request);
        return;
    }
    
    int amount = json_object_get_int(amount_obj);
    int result = kv_increase(&kv, (char*)key, amount);
    
    json_object* response_obj = json_object_new_object();
    if (result == 0) {
        json_object_object_add(response_obj, "message", json_object_new_string("Value incremented successfully"));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 200, "application/json", json_str);
    } else {
        json_object_object_add(response_obj, "error", json_object_new_string("Failed to increment value"));
        const char* json_str = json_object_to_json_string(response_obj);
        send_response(client_socket, 500, "application/json", json_str);
    }
    
    json_object_put(response_obj);
    json_object_put(request);
}

// Handle DELETE /api/keys/{key} - Delete key
void handle_delete_key(int client_socket, const char* key) {
    kv_delete(&kv, (char*)key);
    
    json_object* response_obj = json_object_new_object();
    json_object_object_add(response_obj, "message", json_object_new_string("Key deleted successfully"));
    const char* json_str = json_object_to_json_string(response_obj);
    send_response(client_socket, 200, "application/json", json_str);
    
    json_object_put(response_obj);
}

// Main HTTP request handler
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);
    
    // Parse request line
    char method[10], path[100], version[10];
    sscanf(buffer, "%s %s %s", method, path, version);
    
    // Handle CORS preflight
    if (strcmp(method, "OPTIONS") == 0) {
        send_response(client_socket, 200, "application/json", "");
        return;
    }
    
    // Route requests
    if (strcmp(path, "/api/keys") == 0) {
        if (strcmp(method, "GET") == 0) {
            handle_list_keys(client_socket);
        } else if (strcmp(method, "POST") == 0) {
            handle_create_key(client_socket, buffer);
        }
    } else if (strncmp(path, "/api/keys/", 10) == 0) {
        char key[31];
        strncpy(key, path + 10, 30);
        key[30] = '\0';
        
        char* increment_suffix = strstr(key, "/increment");
        if (increment_suffix != NULL) {
            *increment_suffix = '\0';  // Remove /increment from key
            if (strcmp(method, "PUT") == 0) {
                handle_increment_key(client_socket, key, buffer);
            }
        } else {
            if (strcmp(method, "GET") == 0) {
                handle_get_key(client_socket, key);
            } else if (strcmp(method, "DELETE") == 0) {
                handle_delete_key(client_socket, key);
            }
        }
    } else {
        send_response(client_socket, 404, "application/json", "{\"error\":\"Not found\"}");
    }
}

int start_http_server() {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    
    printf("HTTP server started on port %d\n", PORT);
    
    while (1) {
        int client_socket;
        int addrlen = sizeof(address);
        
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            continue;
        }
        
        handle_request(client_socket);
        close(client_socket);
    }
    
    return 0;
}
