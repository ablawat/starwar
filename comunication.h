#ifndef COMUNICATION_H
#define COMUNICATION_H

typedef struct movement
{
    int object_type;
    int object_id;
    int direction;
}
Movement;

typedef struct enemies
{
    int   size;
    Enemy enemies;
}
Enemies;

#endif