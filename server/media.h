#ifndef MEDIA_H
#define MEDIA_H

#include <stddef.h>

typedef struct app_vars app_vars;

void create_app_vars(app_vars **vars); 
void init_app_vars(app_vars *vars); 
void destroy_app_vars(app_vars **vars); 
size_t get_app_vars_size(); 
void server_create_app_vars(void **vars, size_t max_clients); 
void server_destroy_app_vars(void **vars); 
size_t serialize(char buffer[4096], void *data, size_t amount); 

#endif
