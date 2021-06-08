#ifndef MEDIA_H
#define MEDIA_H

#include <stddef.h>
#include <stdbool.h>

typedef struct app_vars app_vars;
typedef struct media media;

void create_app_vars(app_vars **vars); 
void init_app_vars(app_vars *vars); 
void destroy_app_vars(app_vars **vars); 
size_t get_app_vars_size(); 
void server_create_app_vars(void **vars, size_t max_clients); 
void server_destroy_app_vars(void **vars); 
void create_media(media **m, size_t max_clients); 
void destroy_media(media **m); 
bool is_running(media *m); 
void get_input(media *m); 

void mark_loop_beginning(media *m); 
void mark_loop_duration(media *m); 
void delay_loop(media *m); 
void draw(media *m, app_vars *vars); 
size_t get_data_size(); 
void *get_data(media *m); 
size_t serialize(char buffer[4096], void *data, size_t amount); 
void deserialize(char buffer[4096], void *data, size_t amount); 

#endif
