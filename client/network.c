#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "network.h"
#include "media.h"

#define PORT 25555

struct network_vars {
    int socket; 
    size_t max_clients, data_size; 
    struct sockaddr_in addr;
    void *server_data, *client_data;
    pthread_mutex_t *lock;
    pthread_cond_t *cond;
};


void *sendmessage(void *args) {
    network_vars *net_vars = (network_vars *) args;
    ssize_t bytes_sent;
    size_t buffer_size;
    char buffer[4096];
    char *change = net_vars->client_data; //TODO do for all
    int socket = net_vars->socket;

    do {
        pthread_mutex_lock((net_vars->lock));
        if (*(bool *) change) {
            buffer_size = serialize(buffer, net_vars->client_data, 1);
            bytes_sent = send(socket, buffer, buffer_size, 0);
            printf("Bytes sent %ld\n", bytes_sent);
        }
        pthread_cond_wait(net_vars->cond, net_vars->lock);
        pthread_mutex_unlock((net_vars->lock));
    } while (1); // TODO

    //close(my_socket);
    pthread_exit(NULL);
}

void *listener(void *args) {
    network_vars *net_vars = (network_vars *) args;
    ssize_t bytes_received;
    size_t buffer_size;
    char buffer[4096];
    int socket = net_vars->socket;

    pthread_mutex_lock((net_vars->lock));
    buffer_size = serialize(buffer, net_vars->server_data, net_vars->max_clients); 
    pthread_mutex_unlock((net_vars->lock));

    do {
        bytes_received = recv(socket, buffer, buffer_size, 0);
        pthread_mutex_lock((net_vars->lock));
        deserialize(buffer, net_vars->server_data, net_vars->max_clients);
        printf("Bytes received %ld\n", bytes_received);
        pthread_cond_wait(net_vars->cond, net_vars->lock);
        pthread_mutex_unlock((net_vars->lock));
    } while (1);

    pthread_exit(NULL);
}

void set_app_data(network_vars *net_vars, void *clients_data, void *server_data, size_t data_size) {
    net_vars->client_data = clients_data;
    net_vars->data_size = data_size;
    net_vars->server_data = server_data;
    return;
}

void create_network(network_vars **net_vars, size_t max_clients) {
    (*net_vars) = calloc(1, sizeof(network_vars));
    (*net_vars)->max_clients = max_clients;
    return;
}

int init_network(network_vars *net_vars, pthread_mutex_t *lock, pthread_cond_t *cond) {
    net_vars->addr.sin_family = AF_INET;
    net_vars->addr.sin_port = htons(PORT);
    net_vars->addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    net_vars->lock = lock;
    net_vars->cond = cond;
    if ((net_vars->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET;
    return OK; 
}

void destroy_network(network_vars **net_vars) {
    free(*net_vars);
    return;
}

int connect_to_server(network_vars *net_vars) {
    if (connect(net_vars->socket, (struct sockaddr *) &(net_vars->addr), sizeof(net_vars->addr)) == -1) 
        return ERROR_CONNECT;
    printf("Connected to server\n");
    return OK;
}

void check_error(int error) {
    if (error == OK)
        return;
    fprintf(stderr, "ERROR ON ");
    switch (error) {
        case ERROR_SOCKET:
            fprintf(stderr, "SOCKET\n");
            break;
        case ERROR_BIND:
            fprintf(stderr, "BIND\n");
            break;
        case ERROR_LISTEN:
            fprintf(stderr, "LISTEN\n");
            break;
        case ERROR_ACCEPT:
            fprintf(stderr, "ACCEPT\n");
            break;
        case ERROR_CONNECT:
            fprintf(stderr, "CONNECT\n");
            break;
    }
    exit(1);
}
