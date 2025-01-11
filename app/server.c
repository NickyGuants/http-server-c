#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	 int server_fd, client_addr_len;
         struct sockaddr_in client_addr;
	
	 server_fd = socket(AF_INET, SOCK_STREAM, 0);
	 if (server_fd == -1) {
	 	printf("Socket creation failed: %s...\n", strerror(errno));
	 	return 1;
	 }
	
	// // Since the tester restarts your program quite often, setting SO_REUSEADDR
	// // ensures that we don't run into 'Address already in use' errors
	 int reuse = 1;
	 if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
	 	printf("SO_REUSEADDR failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
	 								 .sin_port = htons(4221),
	 								 .sin_addr = { htonl(INADDR_ANY) },
	 								};
	
	 if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
	 	printf("Bind failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	 int connection_backlog = 5;
	 if (listen(server_fd, connection_backlog) != 0) {
	 	printf("Listen failed: %s \n", strerror(errno));
	 	return 1;
	 }
	
	 printf("Waiting for a client to connect...\n");
	 client_addr_len = sizeof(client_addr);
	
	 int connected_socket = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	 printf("Client connected\n");

	 // Receive Request
	 char buffer[1024] = {0};
	 int received = recv(connected_socket, buffer, sizeof(buffer)-1, 0);
	 if(received > 0){
		printf("Received request: %s\n", buffer);
	 }
	 
	// Extract the Request Line
	 char* end_of_line = strstr(buffer, "\r\n");
	 char request_line[256] = {0};
	 if(end_of_line){
		strncpy(request_line, buffer, end_of_line-buffer);
		printf("Request Line: %s\n", request_line);
	 }
	
	//Parse the Request Line
	
	 char method[16], url[256], version[16];
	 if(sscanf(request_line, "%15s %255s %15s", method, url, version) == 3){
		printf("Method: %s, URL: %s, version: %s\n", method, url, version);
	 }

	 const char *response;
	 if(strcmp(url, "/") == 0){
	 	response = "HTTP/1.1 200 OK\r\n\r\n";
	 }else if(strncmp(url, "/echo/", 6) == 0){
		const char *echo_str = url +6;
		char response_buffer[1024];
		int content_length = strlen(echo_str);
		
		snprintf(response_buffer, sizeof(response_buffer), "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: %d\r\n\r\n" "%s", content_length, echo_str);
		response = response_buffer;	
		
	 }else if(strcmp(url, "/user-agent")==0){
		char *user_agent_start = strstr(buffer, "User-Agent: ");
		if(user_agent_start){
			user_agent_start += strlen("User-Agent: "); // move to the start of the user agent content
			char *user_agent_end = strstr(user_agent_start, "\r\n"); // find the end of the user agent header
			if(user_agent_end){
				char user_agent[256] = {0};
				strncpy(user_agent, user_agent_start, user_agent_end - user_agent_start);
				char response_buffer[1024];
				int content_length = strlen(user_agent);
				snprintf(response_buffer, sizeof(response_buffer), "HTTP/1.1 200 OK\r\n" "Content-Type: text/plain\r\n" "Content-Length: %d\r\n\r\n" "%s", content_length, user_agent);
				response = response_buffer;
			}
		}
	 } else{
		response = "HTTP/1.1 404 Not Found\r\n\r\n";
	 }


	 if(send(connected_socket,response,strlen(response),0) <0){
	 	printf("Message not sent: %s \n", strerror(errno));
	 }  
	 printf("Response sent to client successfully \n");

	 close(connected_socket);
	 close(server_fd);

	return 0;
}
