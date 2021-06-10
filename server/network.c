#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "network.h"
#include "media.h"

#define PORT 25555

struct network_vars {
    int server_socket, *clients_sockets; 
    size_t max_clients, current_client, clients_data_size; 
    struct sockaddr_in addr;
    //void *clients_data;
    char clients_data[4096];
    pthread_mutex_t lock;
};

void set_clients_data(network_vars *net_vars, void *clients_data) {
    //net_vars->clients_data = clients_data;
    //memcpy(net_vars->clients_data, clients_data, client_data_size*net_vars->max_clients);
    net_vars->clients_data_size = serialize(net_vars->clients_data, clients_data, net_vars->max_clients);
    return;
}

void *sendmessage(void *args) {
    network_vars *net_vars = (network_vars *) args;
    size_t current_client = net_vars->current_client;
    size_t data_size = net_vars->clients_data_size; //TODO mudar para quantidade atual
    printf("%ld client\n", current_client);
    int sock_client = net_vars->clients_sockets[current_client];
    ssize_t bytes_sent;
    bool to_send = false;

    char *change[net_vars->max_clients];
    for (size_t i = 0; i < net_vars->max_clients; i++) 
        change[i] = net_vars->clients_data + i* (data_size / net_vars->max_clients);

    bytes_sent = send(sock_client, net_vars->clients_data, data_size, 0);
    do {
        //pthread_mutex_lock(&(net_vars->lock));
        for (size_t i = 0; i < net_vars->max_clients; i++) 
            if (*(bool *) change[i]) {
                (*(bool *) change[i]) = false;
                to_send = true;
            }
        if (to_send) {
            bytes_sent = send(sock_client, net_vars->clients_data, data_size, 0);
            to_send = false;
        }

        /*
        if (*((bool *) change)) {
            (*(bool *) change) = false;
            printf("mudou, enviado\n");
            bytes_sent = send(sock_client, net_vars->clients_data, data_size, 0);
            printf("%ld sent\n", bytes_sent);
        }
        */
        //pthread_mutex_unlock(&(net_vars->lock));
        //usleep(100);
    } while (1); // TODO

    //close(my_socket);
    pthread_exit(NULL);
}

void *listener(void *args) {
    network_vars *net_vars = (network_vars *) args;
    int sock_client = net_vars->clients_sockets[net_vars->current_client];
    size_t current_client = net_vars->current_client;
    size_t data_size = net_vars->clients_data_size / net_vars->max_clients;
    ssize_t bytes_received;
    char *data = net_vars->clients_data + (data_size*current_client);

    do {
        //pthread_mutex_lock(&(net_vars->lock));
        bytes_received = recv(sock_client, data, data_size, 0);
        printf("%ld received\n", bytes_received);
        //pthread_mutex_unlock(&(net_vars->lock));
    } while (1);
    pthread_exit(NULL);
}

void create_network(network_vars **net_vars, size_t max_clients) {
    (*net_vars) = calloc(1, sizeof(network_vars));
    (*net_vars)->clients_sockets = calloc(max_clients, sizeof(int));
    (*net_vars)->max_clients = max_clients;
    return;
}

int init_network(network_vars *net_vars) {
    net_vars->addr.sin_family = AF_INET;
    net_vars->addr.sin_port = htons(PORT);
    net_vars->addr.sin_addr.s_addr = INADDR_ANY;
    pthread_mutex_init(&(net_vars->lock), NULL);
    if ((net_vars->server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET;
    if (bind(net_vars->server_socket, (struct sockaddr *) &net_vars->addr, sizeof(net_vars->addr)) == -1)
        return ERROR_BIND;
    if (listen(net_vars->server_socket, (int) net_vars->max_clients) == -1)
        return ERROR_LISTEN;
    return OK; 
}

void destroy_network(network_vars **net_vars) {
    pthread_mutex_destroy(&(*net_vars)->lock);
    free((*net_vars)->clients_sockets);
    free(*net_vars);
    return;
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
    return;
}

int accept_connection(network_vars *net_vars, size_t client_id) {
    if ((net_vars->clients_sockets[client_id] = accept(net_vars->server_socket, 0, 0)) == -1)
        return ERROR_ACCEPT;
    net_vars->current_client = client_id;
    return OK;
}
