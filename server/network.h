#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>
#include <pthread.h>

typedef struct network_vars network_vars;
enum {OK, ERROR_SOCKET, ERROR_BIND, ERROR_LISTEN, ERROR_ACCEPT, ERROR_CONNECT};

void create_network(network_vars **net_vars, size_t max_clients); 
int init_network(network_vars *net_vars, pthread_t *threads, pthread_attr_t *attr); 
void destroy_network(network_vars **net_vars); 
int check_error(int error); 
int accept_connection(network_vars *net_vars, size_t client_id); 
void *listener(void *args); 
void *sendmessage(void *args); 
void *wait_for_connections(void *args); 
void set_clients_data(network_vars *net_vars, void *clients_data); 
size_t get_amount_clients(network_vars *net_vars); 
void stop_connecting(network_vars *net_vars); 
void show_active_clients(network_vars *net_vars); 
void show_data_in_out(network_vars *net_vars); 
void disconnect_all_clients(network_vars *net_vars); 

#endif
