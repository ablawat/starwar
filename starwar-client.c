#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define BUFFER_LEN     64
#define UNIX_PATH_MAX  128

#define WINDOW_X       640
#define WINDOW_Y       480

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
    
    XSelectInput(*display, *window, ExposureMask | KeyPressMask | KeyReleaseMask);
    XMapWindow(*display, *window);
    
    *pixmap = XCreatePixmap(*display, *window, WINDOW_X, WINDOW_Y, 24);
}

void colors_alloc(Display **display, XColor *player_color)
{
    Colormap colormap = DefaultColormap(*display, DefaultScreen(*display));
    XColor dummy;
    
    XAllocNamedColor(*display, colormap, "cyan", player_color, &dummy);
}

int main()
{
    const char server_name[128] = "starwar-test-server";
    
    int client_socket;
    int result;
    int n_events;
    
    char buffer[BUFFER_LEN];
    double numbers[2];
    
    struct sockaddr_un server_addres;
    
    int addres_len;
    
    int player_key;
    
    Display *display;
    Window   window;
    Pixmap   pixmap;
    
    XEvent   event;
    XColor   player_color;
    
    
    server_addres.sun_family = AF_UNIX;
    strcpy(server_addres.sun_path, server_name);
    
    
    client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    
    addres_len = sizeof(struct sockaddr_un);
    
    result = connect(client_socket, (struct sockaddr *)&server_addres, addres_len);
    
    if (result == 0)
    {
        puts("Connection to server established.");
        
        numbers[0] = 20.0;
        numbers[1] = 20.0;
        
        create_window(&display, &window, &pixmap);
        colors_alloc(&display, &player_color);
        
        send(client_socket, (char *)numbers, sizeof(double) * 2, 0);
        
        player_key = 0;
        
        while (1)
        {
            //recv(client_socket, (char *)numbers, BUFFER_LEN, 0);
            
            GC gc = DefaultGC(display, DefaultScreen(display));
            
            XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
            XFillRectangle(display, pixmap, gc, 0, 0, WINDOW_X, WINDOW_Y);
            
            XSetForeground(display, gc, player_color.pixel);
            XFillRectangle(display, pixmap, gc, (int)((WINDOW_X / 40.0) * numbers[0]) - 10, (int)((WINDOW_Y / 30.0) * numbers[1]) - 10, 20, 20);
            
            XCopyArea(display, pixmap, window, gc, 0, 0, WINDOW_X, WINDOW_Y, 0, 0);
            XFlush(display);
            
            usleep(500);
            
            n_events = XEventsQueued(display, QueuedAlready);
            
            if (n_events > 0)
            {
                XNextEvent(display, &event);
                
                if (event.type == KeyPress)
                {
                    KeySym key = XLookupKeysym(& event.xkey, 0);
                    
                    if ( key == XK_Escape)
                    {
                        XCloseDisplay(display);
                        break;
                    }
                    
                    if (player_key == 0)
                    {
                        if (key == XK_Up)
                        {
                            player_key = 1;
                            puts("Key Up pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Down)
                        {
                            player_key = 2;
                            puts("Key Down pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Left)
                        {
                            player_key = 3;
                            puts("Key Left pressed.");
                            fflush(stdout);
                        }
                        else if (key == XK_Right)
                        {
                            player_key = 4;
                            puts("Key Right pressed.");
                            fflush(stdout);
                        }
                    }
                }
                if (event.type == KeyRelease)
                {
                    unsigned short is_retriggered = 0;
                    
                    if (XEventsQueued(display, QueuedAfterReading))
                    {
                        XEvent new;
                        XPeekEvent(display, &new);
                        
                        if (new.type == KeyPress && new.xkey.time == event.xkey.time &&
                            new.xkey.keycode == event.xkey.keycode)
                        {
                            XNextEvent (display, &event);
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
                                puts("Key Up released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Down && player_key == 2)
                            {
                                player_key = 0;
                                puts("Key Down released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Left && player_key == 3)
                            {
                                player_key = 0;
                                puts("Key Left released.");
                                fflush(stdout);
                            }
                            else if (key == XK_Right && player_key == 4)
                            {
                                player_key = 0;
                                puts("Key Right released.");
                                fflush(stdout);
                            }
                        }
                    }
                }
            }
            else
            {
                //usleep(100);
            }
            
            //send(client_socket, (char *)numbers, sizeof(double) * 2, 0);
        }
    }
    else
    {
        puts("Connection to server refused.");
    }
    
    close(client_socket);
    
    return 0;
}