#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "media.h"

enum {X, Y};

struct app_vars {
    int new_mouse_position[2], old_mouse_position[2];
    int line_size;
    bool mouse_clicked;
    bool running;
};

size_t get_app_vars_size() {
    return sizeof(app_vars);
}

void create_app_vars(app_vars **vars) {
    *vars = calloc(1, sizeof(app_vars));
    return;
}

void init_app_vars(app_vars *vars) {
    vars->line_size = 15;
    vars->mouse_clicked = false;
    vars->running = false;
    vars->old_mouse_position[X] = -1;
    vars->old_mouse_position[Y] = -1;
    vars->new_mouse_position[X] = -1;
    vars->new_mouse_position[Y] = -1;
    return;
}

void destroy_app_vars(app_vars **vars) {
    free(*vars);
    *vars = NULL;
    return;
}

void server_create_app_vars(void **vars, size_t max_clients) {
    *vars = calloc(max_clients, sizeof(app_vars));
    app_vars *app = *vars;
    for (size_t i = 0; i < max_clients; i++)
        init_app_vars(&app[i]);
    return;
}

void server_destroy_app_vars(void **vars) {
    free(*vars);
    *vars = NULL;
    return;
}

size_t serialize(char buffer[4096], void *data, size_t amount) {
    size_t byteoff = 0;
    app_vars *app = data;

    for (size_t i = 0; i < amount; i++) {
        memcpy(buffer + byteoff, app[i].old_mouse_position, sizeof(int)*2); 
        byteoff += sizeof(int)*2;
        memcpy(buffer + byteoff, app[i].new_mouse_position, sizeof(int)*2); 
        byteoff += sizeof(int)*2;
        memcpy(buffer + byteoff, &app[i].mouse_clicked, sizeof(bool)); 
        byteoff += sizeof(bool);
        memcpy(buffer + byteoff, &app[i].line_size, sizeof(int)); 
        byteoff += sizeof(int);
        memcpy(buffer + byteoff, &app[i].running, sizeof(bool)); 
        byteoff += sizeof(bool);
    }
    printf("%ld\n", byteoff);
    return byteoff;
}

