#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LEN     64
#define UNIX_PATH_MAX  128

int main()
{
    const char server_name[128] = "starwar-test-server";
    
    int socket1;
    int socket2;
    
    char buffer[BUFFER_LEN];
    double numbers[2];
    
    struct sockaddr_un server_addres;
    struct sockaddr_un client_addres;
    
    int addres_len;
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    
    socket1 = socket(AF_UNIX, SOCK_STREAM, 0);
    
    bind(socket1, (struct sockaddr *)&server_addres, offsetof(struct sockaddr_un, sun_path) + strlen(server_addres.sun_path));
    
    listen(socket1, 10);
    
    puts("Waiting for connection...");
    
    addres_len = sizeof(struct sockaddr_un);
    socket2 = accept(socket1, (struct sockaddr *)&client_addres, &addres_len);
    
    puts("Client connected.");
    
    while (1)
    {
        recv(socket2, (char *)numbers, BUFFER_LEN, 0);
        //puts("recv");
        //fflush(stdout);
        
        //usleep(2000);
        
        send(socket2, (char *)numbers, sizeof(double) * 2, 0);
        //puts("send");
        //fflush(stdout);
    }
    
    puts("Connection closed.");
    
    close(socket2);
    close(socket1);
    unlink(server_name);
    
    return 0;
}