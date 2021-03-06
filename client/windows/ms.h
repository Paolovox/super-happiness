/*
 * ms.h
 *
 *  Created on: Aug 10, 2016
 *      Author: and22
 */

#ifndef DEBUG_CLIENT_WINDOWS_MS_H_
#define DEBUG_CLIENT_WINDOWS_MS_H_

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdbool.h>
#include "../utils.h"


SOCKET create_socket() {
	//socket
	SOCKET sock;
	//initializing win socket structure
	WSADATA wsa;
	printf("Initializing windows socket structure... ");
	if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		puts("Error in Windows socket structure initialization");
		exit(-1);
	}
	puts("[OK]");
	printf("Creating socket... ");
	//create socket
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		puts("Error while creating socket...");
		exit(-1);
	}
	puts("[OK]");
	//socket successfully created
	return sock;
}

//converting ip address to struct in_addr suitable format
void convert_ip_to_address(const char* ip, struct sockaddr_in* addr) {
	//converting ip address in dotted notation
	//to a suitable form for struct in_addr
	addr->sin_addr.S_un.S_addr =  inet_addr(ip);
	if(addr->sin_addr.S_un.S_addr == INADDR_NONE) {
		puts("Error in converting ip address");
		exit(-1);
	}
}

//read file lines, returning a pointer to pointer
conf_file_t* read_lines(FILE* conf_file) {
	//path structure array
	conf_file_t* paths;
	//array index-iterator
	int i, j, byte_read, line_size = 100, num_lines = 100;
	//temp buffer for reading character
	char* temp;
    //initializing pointer to conf file struct
    paths = malloc(sizeof(conf_file_t*));
	//initializing number of lines (100 lines)
    paths->paths = malloc(sizeof(path_t*)*100);
	//read file line by line
	for(i = 0; ; i++) {
		if(i > num_lines) {
			num_lines += 100;
			//realloc of array size
			realloc(paths->paths, sizeof(path_t*)*num_lines);
		}
		//repristinating line_size
		line_size = 100;
		//initializing path_t structure for corresponding line
		paths->paths[i] = malloc(sizeof(path_t));
		//initializing path variable inside the structure
        paths->paths[i]->path = malloc(sizeof(char)*line_size);
		//iterating over a line
		for(j = 0; ; j++) {
			//exceeding path buffer
			if(j > line_size) {
				//reallocate line size
				line_size += 100;
				realloc(paths->paths[i]->path, sizeof(char)*line_size);
			}
			//reading char by char
			byte_read = fread(paths->paths[i]->path+j, 1, 1, conf_file);
			//recursive path
			if(j == 0 && *(paths->paths[i]->path+j) == '+') {
				//print for debugging
                paths->paths[i]->recusive = true;
			}
			//if new line
			if(*(paths->paths[i]->path+j) == '\r') {
				//substituting end of string to new line
                paths->paths[i]->path[j] = '\0';
				//reading newline character
				byte_read = fread(temp, 1, 1, conf_file);
				//checking whether it's really newline char
				if(*temp == '\n') {
					break;
				}
				else {
					puts("Error while reading newline character");
					exit(-1);
				}
			}
			if(byte_read == 0) {
				//reached end of file
				return paths;
			}
		}
	}
	//function should exit when reading no character
	//if it reaches this point, something went wrong
	printf("[FAIL]\n");
	puts("Error while parsing configuration file ");
	exit(-1);

}

//main function, updates monitoring loop
void monitor_updates(conf_file_t* paths, struct sockaddr_in server) {
	//variables

    //bool for sockopt flag
    BOOL value = TRUE;
	//bytes read
	int bytes, sender_size;
	//socket
	SOCKET sock;
	//receiving buffer
	char buffer[4096];
	//client address info
	struct sockaddr_in client, sender;

	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons((unsigned int)9090);
	client.sin_family = AF_INET;

	sock = create_socket();

    //setting sockopt to avoid error "Socket in use"
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value));

	//binding socket to 9090 port and server address
	if(bind(sock,(struct sockaddr*) &client, sizeof(client)) != 0){
		perror("");
		exit(-1);
	}

    sender_size = sizeof(sender);

	while(1) {
		//receiving bytes and saving onto temporary struct sockaddr_in
		bytes = recv(sock, buffer, 4090, 0);
		printf("Bytes read: %d\n", bytes);
		if(bytes == -1) {
			printf("%d\n", WSAGetLastError());
			exit(-1);
		}
		if(bytes > 0) {
			buffer[4096] = '\0';
			printf("%s\n", buffer);
		}
	}

}


#endif /* DEBUG_CLIENT_WINDOWS_MS_H_ */
