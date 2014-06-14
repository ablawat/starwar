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

typedef struct enemy
{
    double position_x;
    double position_y;
    
    double moving_speed;
}
Enemy;

#include "constraints.h"
#include "comunication.h"


int main()
{
    const char server_name[128] = "starwar-test-server";
    
    int socket_listen;
    int socket_client1;
    int socket_client2;
    
    int player_id = 0;
    
    char   *comunication_buffer = malloc(BUFFER_LEN);
    size_t  comunication_len;
    
    Enemy *enemies = malloc(sizeof(Enemy) * 2);
    unsigned int n_enemies = 2;
    
    Movement  object_movement;
    Enemies  *all_enemies;
    
    struct sockaddr_un server_addres;
    
    struct sockaddr_un client_addres1;
    struct sockaddr_un client_addres2;
    
    int addres_len;
    int result;
    
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    enemies[0].position_x = 10.0;
    enemies[0].position_y = 10.0;
    enemies[0].moving_speed = 6.0;
    
    enemies[1].position_x = 30.0;
    enemies[1].position_y = 15.0;
    enemies[1].moving_speed = 10.0;
    
    socket_listen = socket(AF_UNIX, SOCK_STREAM, 0);
    
    bind(socket_listen, (struct sockaddr *)&server_addres, offsetof(struct sockaddr_un, sun_path) + strlen(server_addres.sun_path));
    
    listen(socket_listen, 10);
    
    addres_len = sizeof(struct sockaddr_un);
    
    puts("Waiting for connection...");
    socket_client1 = accept(socket_listen, (struct sockaddr *)&client_addres1, &addres_len);
    puts("Client 1 connected.");
    
    send(socket_client1, (char *)&player_id, sizeof(int), 0);
    
    player_id++;
    
    puts("Waiting for connection...");
    socket_client2 = accept(socket_listen, (struct sockaddr *)&client_addres2, &addres_len);
    puts("Client 2 connected.");
    
    send(socket_client2, (char *)&player_id, sizeof(int), 0);
    
    all_enemies = (Enemies *)comunication_buffer;
    all_enemies -> size = n_enemies;
    comunication_len = sizeof(Enemy) * n_enemies;
    memcpy(&(all_enemies -> enemies), enemies, comunication_len);
    comunication_len += sizeof(int);
    
    send(socket_client1, comunication_buffer, comunication_len, 0);
    send(socket_client2, comunication_buffer, comunication_len, 0);
    
    int blocking = 1;
    
    ioctl(socket_client1, FIONBIO, &blocking);
    ioctl(socket_client2, FIONBIO, &blocking);
    
    while (1)
    {
        result = recv(socket_client1, (char *)&object_movement, sizeof(Movement), 0);
        
        if (result == sizeof(Movement))
        {
            send(socket_client2, (char *)&object_movement, sizeof(Movement), 0);
            
            //puts("recv 1");
            //fflush(stdout);
        }
        else if (result == 0)
        {
            close(socket_client1);
            close(socket_client2);
            
            puts("Client 1 disconnected.");
            break;
        }
        
        //...
        
        result = recv(socket_client2, (char *)&object_movement, sizeof(Movement), 0);
        
        if (result == sizeof(Movement))
        {
            send(socket_client1, (char *)&object_movement, sizeof(Movement), 0);
            
            //puts("recv 2");
            //fflush(stdout);
        }
        else if (result == 0)
        {
            close(socket_client1);
            close(socket_client2);
            
            puts("Client 2 disconnected.");
            break;
        }
        
        //usleep(100);
    }
    
    puts("Connection closed.");
    
    close(socket_listen);
    unlink(server_name);
    
    return 0;
}
