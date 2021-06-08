#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

typedef struct network_vars network_vars;
enum {OK, ERROR_SOCKET, ERROR_BIND, ERROR_LISTEN, ERROR_ACCEPT, ERROR_CONNECT};

void create_network(network_vars **net_vars, size_t max_clients); 
int init_network(network_vars *net_vars); 
void destroy_network(network_vars **net_vars); 
void check_error(int error); 
int accept_connection(network_vars *net_vars, size_t client_id); 
void *listener(void *args); 
void *sendmessage(void *args); 
void set_clients_data(network_vars *net_vars, void *clients_data); 

#endif
