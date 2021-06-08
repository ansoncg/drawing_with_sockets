#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <pthread.h>

typedef struct network_vars network_vars;
enum {OK, ERROR_SOCKET, ERROR_BIND, ERROR_LISTEN, ERROR_ACCEPT, ERROR_CONNECT};

void create_network(network_vars **net_vars, size_t max_clients); 
int init_network(network_vars *net_vars, pthread_mutex_t *lock); 
void destroy_network(network_vars **net_vars); 
void check_error(int error); 
void *listener(void *args); 
void *sendmessage(void *args); 
int connect_to_server(network_vars *net_vars); 
void *get_server_data(network_vars *net_vars); 
void set_client_data(network_vars *net_vars, void *clients_data, size_t data_size); 

void locknet(network_vars *net_vars); 
void unlocknet(network_vars *network_vars); 
#endif
