#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>

#include "constraints.h"
#include "comunication.h"

int main()
{
    const char server_name[128] = "starwar-test-server";
    
    int socket_listen;
    int socket_client1;
    int socket_client2;
    
    char buffer[BUFFER_LEN];
    double position[2];
    
    Movement player_move;
    
    struct sockaddr_un server_addres;
    
    struct sockaddr_un client_addres1;
    struct sockaddr_un client_addres2;
    
    int addres_len;
    int result;
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    
    socket_listen = socket(AF_UNIX, SOCK_STREAM, 0);
    
    bind(socket_listen, (struct sockaddr *)&server_addres, offsetof(struct sockaddr_un, sun_path) + strlen(server_addres.sun_path));
    
    listen(socket_listen, 10);
    
    addres_len = sizeof(struct sockaddr_un);
    
    puts("Waiting for connection...");
    socket_client1 = accept(socket_listen, (struct sockaddr *)&client_addres1, &addres_len);
    puts("Client 1 connected.");
    
    puts("Waiting for connection...");
    socket_client2 = accept(socket_listen, (struct sockaddr *)&client_addres2, &addres_len);
    puts("Client 2 connected.");
    
    int blocking = 1;
    
    ioctl(socket_client1, FIONBIO, &blocking);
    ioctl(socket_client2, FIONBIO, &blocking);
    
    while (1)
    {
        result = recv(socket_client1, (char *)&player_move, BUFFER_LEN, 0);
        
        if (result == sizeof(Movement))
        {
            player_move.player_id = 1;
            send(socket_client2, (char *)&player_move, sizeof(Movement), 0);
            
            puts("recv 1");
            fflush(stdout);
        }
        else if (result == 0)
        {
            close(socket_client1);
            close(socket_client2);
            
            puts("Client 1 disconnected.");
            break;
        }
        
        //...
        
        result = recv(socket_client2, (char *)&player_move, sizeof(Movement), 0);
        
        if (result == sizeof(Movement))
        {
            player_move.player_id = 2;
            send(socket_client1, (char *)&player_move, sizeof(Movement), 0);
            
            puts("recv 2");
            fflush(stdout);
        }
        else if (result == 0)
        {
            close(socket_client1);
            close(socket_client2);
            
            puts("Client 2 disconnected.");
            break;
        }
        
        usleep(100);
    }
    
    puts("Connection closed.");
    
    close(socket_listen);
    unlink(server_name);
    
    return 0;
}