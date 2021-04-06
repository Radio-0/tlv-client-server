#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "data_rd.h"

#define BUF_SIZE 2048
#define NUMB_COONECT 10

extern int get_str_data(FILE *fd);

int main(int argc, char ** argv)
{
    int sock, newsock;  // Sockets
    int len_struct;     // Size struct 
    int port;           // Port number
    int fw;             // Number of bytes written to the file
    int str_data;        // Return of get_str_data
    uint16_t rec;       // Number of bytes received
    char *file_path;
    uint8_t buf[BUF_SIZE];  // Received data
    struct sockaddr_in serv_addr, client_addr;
    if (argc < 3){
        fprintf(stderr,"usage: %s <port_number>\n", argv[0]);
        return -1;
    }

    // Get port number
    if(!strcmp(argv[1], "-p")){
        port = atoi(argv[2]); 
        if(port < 0 || port > 65536){
            printf("Error port\n");
            return -1;
        }
    }

    // Socket creation
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0){
       printf("socket() failed: %d\n", errno);
       return -1;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Bind a socket to an address
    if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        printf("bind() failed: %s\n", strerror(errno));
        return -1;
    }

    // Maximum numbers of connection
    listen(sock, NUMB_COONECT);
    len_struct = sizeof(client_addr);
    
    while(1){
        /* Waiting for an incoming connection.
        Create a new socket for each connection.*/
        newsock = accept(sock, (struct sockaddr *) &client_addr, &len_struct);
        if (newsock < 0){
            printf("accept() failed: %s\n", strerror(errno));
            close(sock);
            return -1;
        }
        printf("\nCONNECTED\n");
        memset(buf, 0, BUF_SIZE);

        // Receiving data from the client socket
        rec = recv(newsock, buf, BUF_SIZE, 0);

        file_path = (char *) malloc(sizeof(char) * 1024);

        if (file_path == NULL){
            printf("Error memory\n");
            return -1;
        }

        strcpy(file_path, "/home/semin/sc/client-server/server/tlv.bin");
        FILE *fd = fopen(file_path, "wb+");
        if(!fd){
            printf("Error file\n");
            free(file_path);
            return -1;
        }

        fseek(fd, 0, SEEK_SET);

        fw = fwrite(buf, sizeof(uint8_t), rec, fd);
        if(rec != fw)
            printf("File write error\n");
        else 
            printf("File is wrote\n");

        // Reading data from a file and displaying it on the screen (rd_bin.c)
        str_data = get_str_data(fd);
        buf[BUF_SIZE] = 0;
        if(str_data == 0)
            send(newsock, "OK", 2, 0);
        else send(newsock, "ERROR", 5, 0);
        fclose(fd);
    }
    free(file_path);
    close(newsock); 
    close(sock);
    return 0;
}