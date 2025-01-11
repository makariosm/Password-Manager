#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <json-c/json.h>
#include "kvstore.h"

// Main function to start the HTTP server
int start_http_server(void);

// HTTP response helper
void send_response(int client_socket, int status_code, const char* content_type, const char* body);

// Request handlers
void handle_request(int client_socket);
void handle_list_keys(int client_socket);
void handle_get_key(int client_socket, const char* key);
void handle_create_key(int client_socket, char* body);
void handle_increment_key(int client_socket, const char* key, char* body);
void handle_delete_key(int client_socket, const char* key);

// JSON helper
json_object* parse_request_body(char* body);

// Constants
#define HTTP_PORT 8080
#define HTTP_BUFFER_SIZE 1024

#endif // HTTP_SERVER_H