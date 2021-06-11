#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "media.h"

enum {X, Y};
#define FPS 100  

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
    bool change;
    int new_mouse_position[2], old_mouse_position[2];
    int line_size;
    int colour;
    bool mouse_clicked;
    bool running;
};

struct media {
    sdl_vars sdl;  
    app_consts consts; 
    app_vars *local_vars, *server_vars;
    size_t max_clients;
};

void update_mouse_position(SDL_Event event, int mouse_position[2]) {
    mouse_position[X] = event.motion.x;
    mouse_position[Y] = event.motion.y;
    return;
}

void get_input(media *m) {
    app_vars *vars = (m->local_vars);
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
                vars->change = true;
                break;
            case SDL_MOUSEBUTTONUP:
                vars->mouse_clicked = false;
                vars->change = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.scancode >= 30 && event.key.keysym.scancode <= 35)
                    m->local_vars->colour = (int) event.key.keysym.scancode - 29;
                else if (event.key.keysym.scancode == SDL_SCANCODE_Q)
                    m->local_vars->line_size++;
                else if (event.key.keysym.scancode == SDL_SCANCODE_W)
                    m->local_vars->line_size--;
                break;
            default:
                vars->old_mouse_position[X] = vars->old_mouse_position[Y] = -1;
        }
    }
    return;
}
        
void set_colour(sdl_vars *sdl, app_vars *vars) {
    switch (vars->colour) {
        case 1:
            SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
            break;
        case 2:
            SDL_SetRenderDrawColor(sdl->renderer, 255, 0, 0, 255);
            break;
        case 3:
            SDL_SetRenderDrawColor(sdl->renderer, 0, 255, 0, 255);
            break;
        case 4:
            SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 255, 255);
            break;
        case 5:
            SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255);
            break;
    }
}

void mouse_draw(sdl_vars *sdl, app_vars *vars) {
    int lsize = vars->line_size;
    if (vars->mouse_clicked) {
        if (vars->old_mouse_position[X] != -1) {
            set_colour(sdl, vars);
            for (int i = 0; i < lsize; i++)
                for (int j = 0; j < lsize; j++)
                    SDL_RenderDrawLine(sdl->renderer, vars->old_mouse_position[X] + i, vars->old_mouse_position[Y] + j, vars->new_mouse_position[X] + i, vars->new_mouse_position[Y] + j);
        }
        else
            SDL_RenderDrawPoint(sdl->renderer, vars->new_mouse_position[X], vars->new_mouse_position[Y]);
    }
    return;
}

void draw(media *m) {
    app_vars *vars = m->server_vars;

    // Local drawing
    //mouse_draw(&(m->sdl), (m->local_vars));

    // Drawing from the server
    for (size_t i = 0; i < m->max_clients; i++) {
        if (vars[i].running) {
            mouse_draw(&(m->sdl), &vars[i]);
        }
    }
    SDL_RenderPresent(m->sdl.renderer);
    return;
}

void print_app(void *app) {
    app_vars *vars = app;
    printf("%s\n", vars->mouse_clicked ? "CLICKED" : "NOT CLICKED");
    printf("Old mouse: %d %d\n", vars->old_mouse_position[X], vars->old_mouse_position[Y]);
    printf("New mouse: %d %d\n", vars->new_mouse_position[X], vars->new_mouse_position[Y]);
    printf("Line %d\n", vars->line_size);
    return;
}

bool init_sdl(sdl_vars *sdl, app_consts *consts) { 
    if (SDL_Init(SDL_INIT_EVERYTHING))
        return false;
    if(!(sdl->window = SDL_CreateWindow(consts->name, 0, 0, consts->window_width, consts->window_hight, false))) 
        return false;
    if(!(sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_SOFTWARE))) // With flag == 0 I had some problems 
        return false;
    SDL_SetRenderDrawColor(sdl->renderer, 255, 255, 255, 255); 
    sdl->delay = (1000 / FPS);
    SDL_RenderClear(sdl->renderer);
    SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);
    printf("SDL Init\n");
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

void *get_local_data(media *m) {
    return m->local_vars; 
}

void *get_server_data(media *m) {
    return m->server_vars;
}

size_t get_data_size() {
    return sizeof(app_vars);
}

void create_app_vars(app_vars **vars) {
    *vars = calloc(1, sizeof(app_vars));
    return;
}

void init_app_vars(app_vars *vars) {
    vars->change = false;
    vars->line_size = 15;
    vars->mouse_clicked = false;
    vars->running = true;
    vars->old_mouse_position[X] = -1;
    vars->old_mouse_position[Y] = -1;
    vars->new_mouse_position[X] = -1;
    vars->new_mouse_position[Y] = -1;
    vars->colour = 1;
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
        memcpy(buffer + byteoff, &app[i].change, sizeof(bool)); 
        byteoff += sizeof(bool);
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
        memcpy(buffer + byteoff, &app[i].colour, sizeof(bool)); 
        byteoff += sizeof(int);
    }
    return byteoff;
}

void deserialize(char buffer[4096], void *data, size_t amount) {
    size_t byteoff = 0;
    app_vars *app = data;

    for (size_t i = 0; i < amount; i++) {
        memcpy(&app[i].change, buffer + byteoff, sizeof(bool)); 
        byteoff += sizeof(bool);
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
        memcpy(&app[i].colour, buffer + byteoff, sizeof(bool)); 
        byteoff += sizeof(int);
    }
    return;
}

void create_media(media **m, size_t max_clients) {
    *m = calloc(1, sizeof(media));
    (*m)->server_vars = calloc(max_clients, sizeof(app_vars));
    (*m)->local_vars = calloc(1, sizeof(app_vars));
    (*m)->max_clients = max_clients;
    return;
}

void init_media(media *m) {
    strcpy(m->consts.name, "Drawing with sockets");
    m->consts.window_hight = 400;
    m->consts.window_width = 400;
    for (size_t i = 0; i < m->max_clients; i++)
        init_app_vars(&(m->server_vars[i]));
    init_app_vars((m->local_vars));
    m->local_vars->running = init_sdl(&(m->sdl), &(m->consts));
    return;
}

bool is_running(media *m) {
    return m->local_vars->running;
}

void destroy_media(media **m) {
    destroy_sdl(&(*m)->sdl);
    free((*m)->server_vars);
    free((*m)->local_vars);
    free(*m);
    *m = NULL;
    return;
}
