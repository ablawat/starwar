#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define BUFFER_LEN     64
#define UNIX_PATH_MAX  128

int main()
{
    const char server_name[128] = "starwar-server";
    
    int client_socket;
    int result;
    
    char buffer[BUFFER_LEN];
    
    struct sockaddr_un server_addres;
    
    int addres_len;
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
    addres_len = sizeof(struct sockaddr_un);
    
    result = connect(client_socket, (struct sockaddr *)&server_addres, addres_len);
    
    if (result == 0)
    {
        puts("Connection to server established.");
        
        strcpy(buffer, "abcd");
        
        send(client_socket, buffer, strlen(buffer), 0);
        printf("Send    : %s\n", buffer);
        
        recv(client_socket, buffer, BUFFER_LEN, 0);
        printf("Receive : %s\n", buffer);
    }
    else
    {
        puts("Connection to server refused.");
    }
    
    close(client_socket);
    
    return 0;
}