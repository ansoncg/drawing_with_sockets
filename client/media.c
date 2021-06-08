#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "media.h"

enum {X, Y};
#define FPS 60

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    unsigned int delay, frame_start, frame_duration;
} sdl_vars;

typedef struct {
    int window_hight, window_width;
    char name[256];
} app_consts;

struct app_vars {
    int new_mouse_position[2], old_mouse_position[2];
    int line_size;
    bool mouse_clicked;
    bool running;
};

struct media {
    sdl_vars sdl;  
    app_consts consts; 
    app_vars *vars;
    size_t app_vars_size;
};

void update_mouse_position(SDL_Event event, int mouse_position[2]) {
    mouse_position[X] = event.motion.x;
    mouse_position[Y] = event.motion.y;
    return;
}

//void get_input(app_vars *vars) {
void get_input(media *m) {
    app_vars *vars = m->vars;
    SDL_Event event;
    vars->old_mouse_position[X] = vars->new_mouse_position[X];
    vars->old_mouse_position[Y] = vars->new_mouse_position[Y];
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                vars->running = false;
                break;
            case SDL_MOUSEMOTION:
                update_mouse_position(event, vars->new_mouse_position);
                break;
            case SDL_MOUSEBUTTONDOWN:
                vars->mouse_clicked = true;
                break;
            case SDL_MOUSEBUTTONUP:
                vars->mouse_clicked = false;
                break;
            default:
                vars->old_mouse_position[X] = vars->old_mouse_position[Y] = -1;
                vars->mouse_clicked = false;
        }
    }
    return;
}
        
void mouse_draw(sdl_vars *sdl, app_vars *vars) {
    int lsize = vars->line_size;
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
    if (vars->mouse_clicked) {
        if (vars->old_mouse_position[X] != -1) {
            for (int i = 0; i < lsize; i++)
                for (int j = 0; j < lsize; j++)
                    SDL_RenderDrawLine(sdl->renderer, vars->old_mouse_position[X] + i, vars->old_mouse_position[Y] + j, vars->new_mouse_position[X] + i, vars->new_mouse_position[Y] + j);
        }
        else
            SDL_RenderDrawPoint(sdl->renderer, vars->new_mouse_position[X], vars->new_mouse_position[Y]);
    }
    SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255);
    return;
}

void draw(media *m, app_vars *vars) {
    //SDL_RenderClear(sdl->renderer);
    printf("%s\n", vars->mouse_clicked ? "CLICKED" : "NOT CLICKED");
    printf("Old mouse: %d %d\n", vars->old_mouse_position[X], vars->old_mouse_position[Y]);
    printf("New mouse: %d %d\n", vars->new_mouse_position[X], vars->new_mouse_position[Y]);
    printf("Line %d\n", vars->line_size);
    for (size_t i = 0; i < m->app_vars_size; i++) {
        if (vars[i].running) {
            printf("oi %ld\n", i);
            mouse_draw(&(m->sdl), &vars[i]);
            SDL_RenderPresent(m->sdl.renderer);
        }
    }
    return;
}

bool init_sdl(sdl_vars *sdl, app_consts *consts) { 
    if (SDL_Init(SDL_INIT_EVERYTHING))
        return false;
    if(!(sdl->window = SDL_CreateWindow(consts->name, 0, 0, consts->window_width, consts->window_hight, false))) 
        return false;
    if(!(sdl->renderer = SDL_CreateRenderer(sdl->window, -1, 0))) 
        return false;
    SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255); 
    sdl->delay = (1000 / FPS);
    SDL_RenderClear(sdl->renderer);
    printf("SDL certo\n");
    return true;
}

void destroy_sdl(sdl_vars *sdl) {
    SDL_DestroyWindow(sdl->window);
    SDL_DestroyRenderer(sdl->renderer);
    SDL_Quit();
    return;
}

void mark_loop_beginning(media *m) {
    m->sdl.frame_start = SDL_GetTicks();
    return;
}

void mark_loop_duration(media *m) {
    m->sdl.frame_duration = SDL_GetTicks() - m->sdl.frame_start;
    return;
}

void delay_loop(media *m) {
    if (m->sdl.frame_duration < m->sdl.delay)
        SDL_Delay(m->sdl.delay - m->sdl.frame_duration);
    return;
}

void *get_data(media *m) {
    return m->vars; 
}

size_t get_data_size() {
    return sizeof(app_vars);
}

void create_app_vars(app_vars **vars) {
    *vars = calloc(1, sizeof(app_vars));
    return;
}

void init_app_vars(app_vars *vars) {
    vars->line_size = 15;
    vars->mouse_clicked = false;
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

void client_create_app_vars(app_vars **vars, size_t max_clients) {
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
    //printf("%ld\n", byteoff);
    return byteoff;
}

void deserialize(char buffer[4096], void *data, size_t amount) {
    size_t byteoff = 0;
    app_vars *app = data;

    for (size_t i = 0; i < amount; i++) {
        memcpy(app[i].old_mouse_position, buffer + byteoff, sizeof(int)*2); 
        byteoff += sizeof(int)*2;
        memcpy(app[i].new_mouse_position, buffer + byteoff, sizeof(int)*2); 
        byteoff += sizeof(int)*2;
        memcpy(&app[i].mouse_clicked, buffer + byteoff, sizeof(bool)); 
        byteoff += sizeof(bool);
        memcpy(&app[i].line_size, buffer + byteoff, sizeof(int)); 
        byteoff += sizeof(int);
        memcpy(&app[i].running, buffer + byteoff, sizeof(bool)); 
        byteoff += sizeof(bool);
    }
}

void create_media(media **m, size_t max_clients) {
    *m = calloc(1, sizeof(media));
    strcpy((*m)->consts.name, "Drawing with sockets");
    (*m)->consts.window_hight = 600;
    (*m)->consts.window_width = 600;
    client_create_app_vars(&((*m)->vars), max_clients);
    (*m)->vars->running = init_sdl(&(*m)->sdl, &(*m)->consts);
    (*m)->app_vars_size = max_clients;
    return;
}

bool is_running(media *m) {
    return m->vars->running;
}

void destroy_media(media **m) {
    destroy_sdl(&(*m)->sdl);
    free((*m)->vars);
    free(*m);
    return;
}
