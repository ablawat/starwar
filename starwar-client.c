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
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct player
{
    double position_x;
    double position_y;
    
    unsigned int color_index;
    unsigned int score;
}
Player;

typedef struct enemy
{
    double position_x;
    double position_y;
    
    double moving_speed;
}
Enemy;

#include "constraints.h"
#include "comunication.h"

void create_window(Display **display, Window *window, Pixmap *pixmap)
{
    unsigned long background;
    unsigned long border;
    
    int screenNum;
    
    *display = XOpenDisplay(NULL);
    
    screenNum = DefaultScreen(*display);
    background = BlackPixel(*display, screenNum);
    border = WhitePixel(*display, screenNum);
    
    *window = XCreateSimpleWindow(*display, DefaultRootWindow(*display), 0, 0, WINDOW_X, WINDOW_Y, 2, border, background);
    
    XSelectInput(*display, *window, KeyPressMask | KeyReleaseMask);
    XMapWindow(*display, *window);
    
    *pixmap = XCreatePixmap(*display, *window, WINDOW_X, WINDOW_Y, 24);
}

void colors_alloc(Display **display, XColor *colors)
{
    Colormap colormap = DefaultColormap(*display, DefaultScreen(*display));
    XColor dummy;
    
    XAllocNamedColor(*display, colormap, "dodger blue", &colors[0], &dummy);
    XAllocNamedColor(*display, colormap, "dark orange", &colors[1], &dummy);
    XAllocNamedColor(*display, colormap, "sienna", &colors[2], &dummy);
}

int main()
{
    const char server_name[128] = "starwar-test-server";
    
    int client_socket;
    int result;
    int n_events;
    int send_to_server;
    
    int my_id;
    
    struct timespec ts1;
    struct timespec ts2;
    
    double d_time;
    
    char   *comunication_buffer = malloc(BUFFER_LEN);
    size_t  comunication_len;
    
    Enemy *enemies = malloc(sizeof(Enemy) * 2);
    unsigned int n_enemies;
    
    Player players[2];
    
    Movement  object_movement;
    Enemies  *all_enemies;
    
    int is_me_moveing;
    int is_player_moveing;
    
    int is_running;
    int my_move_direction;
    
    const int step_x = WINDOW_X / AREA_WIDTH;
    const int step_y = WINDOW_Y / AREA_HEIGHT;
    
    struct sockaddr_un server_addres;
    
    int addres_len;
    
    int player_key;
    
    int i;
    
    Display *display;
    Window   window;
    Pixmap   pixmap;
    
    XEvent   event;
    XColor   colors[3];
    
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
    addres_len = sizeof(struct sockaddr_un);
    
    result = connect(client_socket, (struct sockaddr *)&server_addres, addres_len);
    
    if (result == 0)
    {
        puts("Connection to server established.");
        
        recv(client_socket, (char *)&my_id, sizeof(int), 0);
        
        recv(client_socket, comunication_buffer, BUFFER_LEN, 0);
        
        all_enemies = (Enemies *)comunication_buffer;
        n_enemies = all_enemies -> size;
        comunication_len = sizeof(Enemy) * n_enemies;
        memcpy(enemies, &(all_enemies -> enemies), comunication_len);
        
        players[0].position_x = 20.0;
        players[0].position_y = 20.0;
        players[0].color_index = 0;
        players[1].position_x = 20.0;
        players[1].position_y = 20.0;
        players[1].color_index = 1;
        
        int blocking = 1;
        
        ioctl(client_socket, FIONBIO, &blocking);
        
        is_me_moveing = 0;
        is_player_moveing = 0;
        
        create_window(&display, &window, &pixmap);
        colors_alloc(&display, colors);
        
        player_key = 0;
        send_to_server = 0;
        
        my_move_direction = 0;
        
        is_running = 1;
        
        while (1)
        {
            clock_gettime(CLOCK_REALTIME, &ts1);
            
            result = recv(client_socket, (char *)&object_movement, sizeof(Movement), 0);
            
            if (result == sizeof(Movement))
            {
                if (object_movement.direction != 0)
                    is_player_moveing = 1;
                else
                    is_player_moveing = 0;
            }
            
            if (is_player_moveing)
            {
                switch (object_movement.direction)
                {
                    case 1: players[object_movement.object_id].position_y -= MOVE_SPEED * d_time;
                            break;
                            
                    case 2: players[object_movement.object_id].position_y += MOVE_SPEED * d_time;
                            break;
                            
                    case 3: players[object_movement.object_id].position_x -= MOVE_SPEED * d_time;
                            break;
                            
                    case 4: players[object_movement.object_id].position_x += MOVE_SPEED * d_time;
                            break;
                }
            }
            
            if (is_me_moveing)
            {
                switch (my_move_direction)
                {
                    case 1: players[my_id].position_y -= MOVE_SPEED * d_time;
                            break;
                            
                    case 2: players[my_id].position_y += MOVE_SPEED * d_time;
                            break;
                            
                    case 3: players[my_id].position_x -= MOVE_SPEED * d_time;
                            break;
                            
                    case 4: players[my_id].position_x += MOVE_SPEED * d_time;
                            break;
                }
            }
            
            GC gc = DefaultGC(display, DefaultScreen(display));
            
            XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
            XFillRectangle(display, pixmap, gc, 0, 0, WINDOW_X, WINDOW_Y);
            
            XSetForeground(display, gc, colors[0].pixel);
            XFillRectangle(display, pixmap, gc, (int)(step_x * players[0].position_x) - 10, (int)(step_y * players[0].position_y) - 10, 20, 20);
            XSetForeground(display, gc, colors[1].pixel);
            XFillRectangle(display, pixmap, gc, (int)(step_x * players[1].position_x) - 10, (int)(step_y * players[1].position_y) - 10, 20, 20);
            
            XSetForeground(display, gc, colors[2].pixel);
            
            for (i = 0; i < n_enemies; i++)
            {
                XFillRectangle(display, pixmap, gc, (int)(step_x * enemies[i].position_x) - 10, (int)(step_y * enemies[i].position_y) - 10, 20, 20);
            }
            
            XCopyArea(display, pixmap, window, gc, 0, 0, WINDOW_X, WINDOW_Y, 0, 0);
            XFlush(display);
            
            usleep(1000);
            
            n_events = XEventsQueued(display, QueuedAlready);
            
            for (i = n_events; i > 4; i--)
            {
                //printf("%d\n", n_events);
                //fflush(stdout);
                
                XNextEvent(display, &event);
                
                if (event.type == KeyPress)
                {
                    KeySym key = XLookupKeysym(& event.xkey, 0);
                    
                    if ( key == XK_Escape)
                    {
                        is_running = 0;
                        break;
                    }
                    
                    if (player_key == 0)
                    {
                        if (key == XK_Up)
                        {
                            player_key = 1;
                            send_to_server = 1;
                            
                            puts("Key Up pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Down)
                        {
                            player_key = 2;
                            send_to_server = 1;
                            
                            puts("Key Down pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Left)
                        {
                            player_key = 3;
                            send_to_server = 1;
                            
                            puts("Key Left pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Right)
                        {
                            player_key = 4;
                            send_to_server = 1;
                            
                            puts("Key Right pressed.");
                            fflush(stdout);
                        }
                    }
                    
                    if (send_to_server)
                    {
                        my_move_direction = player_key;
                        is_me_moveing = 1;
                        
                        object_movement.object_id = my_id;
                        object_movement.direction = player_key;
                        
                        send(client_socket, (char *)&object_movement, sizeof(Movement), 0);
                        send_to_server = 0;
                    }
                }
                if (event.type == KeyRelease)
                {
                    unsigned short is_retriggered = 0;
                    
                    if (XEventsQueued(display, QueuedAfterReading))
                    {
                        XEvent new;
                        XNextEvent(display, &new);
                        
                        if (new.type == KeyPress && new.xkey.time == event.xkey.time &&
                            new.xkey.keycode == event.xkey.keycode)
                        {
                            XNextEvent(display, &event);
                            is_retriggered = 1;
                        }
                    }
                    
                    if (!is_retriggered)
                    {
                        KeySym key = XLookupKeysym(& event.xkey, 0);
                        
                        if (player_key > 0)
                        {
                            if (key == XK_Up && player_key == 1)
                            {
                                player_key = 0;
                                send_to_server = 1;
                                puts("Key Up released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Down && player_key == 2)
                            {
                                player_key = 0;
                                send_to_server = 1;
                                puts("Key Down released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Left && player_key == 3)
                            {
                                player_key = 0;
                                send_to_server = 1;
                                puts("Key Left released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Right && player_key == 4)
                            {
                                player_key = 0;
                                send_to_server = 1;
                                puts("Key Right released.");
                                fflush(stdout);
                            }
                        }
                        
                        if (send_to_server)
                        {
                            my_move_direction = 0;
                            is_me_moveing = 0;
                            
                            object_movement.object_id = my_id;
                            object_movement.direction = 0;
                            
                            send(client_socket, (char *)&object_movement, sizeof(Movement), 0);
                            send_to_server = 0;
                        }
                    }
                }
            }
            
            clock_gettime(CLOCK_REALTIME, &ts2);
            
            d_time = (ts2.tv_sec + ts2.tv_nsec / MLD) - (ts1.tv_sec + ts1.tv_nsec / MLD);
            
            if (is_running == 0)
            {
                break;
            }
        }
    }
    else
    {
        puts("Connection to server refused.");
    }
    
    XCloseDisplay(display);
    close(client_socket);
    
    return 0;
}