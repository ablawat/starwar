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
    
    int socket1;
    int socket2;
    int socket3;
    
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
    
    
    socket1 = socket(AF_UNIX, SOCK_STREAM, 0);
    
    bind(socket1, (struct sockaddr *)&server_addres, offsetof(struct sockaddr_un, sun_path) + strlen(server_addres.sun_path));
    
    listen(socket1, 10);
    
    addres_len = sizeof(struct sockaddr_un);
    
    puts("Waiting for connection...");
    socket2 = accept(socket1, (struct sockaddr *)&client_addres1, &addres_len);
    puts("Client 1 connected.");
    
    puts("Waiting for connection...");
    socket3 = accept(socket1, (struct sockaddr *)&client_addres2, &addres_len);
    puts("Client 2 connected.");
    
    int blocking = 1;
    
    ioctl(socket2, FIONBIO, &blocking);
    ioctl(socket3, FIONBIO, &blocking);
    
    while (1)
    {
        result = recv(socket2, (char *)&player_move, BUFFER_LEN, 0);
        
        if (result == sizeof(Movement))
        {
            player_move.player_id = 1;
            send(socket3, (char *)&player_move, sizeof(Movement), 0);
            
            puts("recv 1");
            fflush(stdout);
        }
        
        //usleep(2000);
        
        result = recv(socket3, (char *)&player_move, sizeof(Movement), 0);
        
        if (result == sizeof(Movement))
        {
            player_move.player_id = 2;
            send(socket2, (char *)&player_move, sizeof(Movement), 0);
            
            puts("recv 2");
            fflush(stdout);
        }
        
        usleep(500);
    }
    
    puts("Connection closed.");
    
    close(socket2);
    close(socket1);
    unlink(server_name);
    
    return 0;
}