#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <netdb.h>
#include "tlv_info.h"
#include "check_ip.h"

#define BUF_SIZE 256

extern tlvinfo_header_t *get_tlv_data(FILE *file);
extern int check_ip();

int main(int argc, char ** argv){
    
    int sock;           // Socket
    int port;           // Port number
    char ip[16];           // IP argv
    FILE *file;         // Transferred file
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buf[BUF_SIZE]; // Received message from server
    int i;              // Counter
    char *file_path = (char *) malloc(sizeof(char) * 1024);    // Path to the file

    if (file_path == NULL){
        printf("Error memory\n");
        return -1;
    }

    if(argc < 6 || argc <= 0){
        fprintf(stderr,"usage: %s <hostname> <port_number> <file_path>\n", argv[0]);
        return -1;
    } else if(!strcmp(argv[1], "--help")){
        printf("You requested help message\n\n");
        printf("-ip \t get IP-address\n");
        printf("-p \t get port number\n");
        printf("-f \t path to the file\n");
        return 0;
    }
    for(i = 1; i < 7; i++){
        if(!strcmp(argv[i], "-ip")){
            
            strncpy(ip, argv[i + 1], strlen(argv[i+1]));
            if(check_ip(ip) != 0){
                printf("Error IP-address\n");
                return -1;
            }
            server = gethostbyname(argv[i + 1]);
        }
        if(!strcmp(argv[i], "-p")){
            port = atoi(argv[i + 1]); 
            if(port < 0 || port > 65536){
                printf("Error port\n");
                return -1;
            }
        }
        if(!strcmp(argv[i], "-f")){
            if (file_path == NULL){
                printf("Error memory\n");
                return -1;
            }
            strncpy(file_path, argv[i+1], strlen(argv[i+1]));
            file = fopen(file_path, "wb+");
            if(!file){
                printf("Error file\n");
                free(file_path);
                return -1;
            }
        }
    }

    // Socket creation
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0){
        printf("socket() failed: %d\n", errno);
        return EXIT_FAILURE;
    }
    if (server == NULL){
        printf("Host not found\n");
        return -1;
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // Get data from eeprom.conf (wr_bin.c)
    static uint8_t data[TLV_INFO_MAX_LEN];
    static tlvinfo_header_t *header = (tlvinfo_header_t *)data;
    header = get_tlv_data(file);
    fclose(file);
    free(file_path);
    printf("File .bin is opened\n");
    
    // Host connection
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("connect() failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    printf(">");

    // Data transfer to the server
    send(sock, header->data, header->datalen, 0);
    memset(buf, 0, BUF_SIZE);

    // Server response
    recv(sock, buf, BUF_SIZE-1, 0);
    printf("%s\n",buf);

    close(sock);
    return 0;
}