#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "network.h"
#include "media.h"

#define PORT 25555

struct network_vars {
    int server_socket, *clients_sockets; 
    size_t max_clients, current_client, clients_data_size, amount_clients; 
    struct sockaddr_in addr;
    char clients_data[4096];
    bool *active_clients;
    bool show_data;
    pthread_mutex_t lock;
    pthread_t *threads;
    pthread_attr_t *attr;
};

void set_clients_data(network_vars *net_vars, void *clients_data) {
    net_vars->clients_data_size = serialize(net_vars->clients_data, clients_data, net_vars->max_clients);
    return;
}

void *sendmessage(void *args) {
    network_vars *net_vars = (network_vars *) args;
    size_t current_client = net_vars->current_client;
    size_t data_size = net_vars->clients_data_size; 
    int sock_client = net_vars->clients_sockets[current_client];
    ssize_t bytes_sent;
    bool to_send = false;
    char *change[net_vars->max_clients];
    bool active = true;

    pthread_mutex_lock(&(net_vars->lock)); // LOCK

    // Storing variables that tells if a change happened
    for (size_t i = 0; i < net_vars->max_clients; i++) 
        change[i] = net_vars->clients_data + i* (data_size / net_vars->max_clients);

    pthread_mutex_unlock(&(net_vars->lock)); // UNLOCK

    do {
        pthread_mutex_lock(&(net_vars->lock)); // LOCK

        // Send loop, reset change variables every send.
        for (size_t i = 0; i < net_vars->max_clients; i++) 
            if (*(bool *) change[i]) {
                (*(bool *) change[i]) = false;
                to_send = true;
            }
        if (to_send) {
            bytes_sent = send(sock_client, net_vars->clients_data, data_size, 0);
            if (net_vars->show_data)
                printf("Bytes sent %ld\n", bytes_sent);
            to_send = false;
        }
        active = net_vars->active_clients[current_client];
        pthread_mutex_unlock(&(net_vars->lock)); // UNLOCK
    } while (active);
    pthread_exit(NULL);
}

void *listener(void *args) {
    network_vars *net_vars = (network_vars *) args;
    int sock_client = net_vars->clients_sockets[net_vars->current_client];
    size_t current_client = net_vars->current_client;
    size_t data_size = net_vars->clients_data_size / net_vars->max_clients;
    ssize_t bytes_received;
    char *data = net_vars->clients_data + (data_size*current_client);
    char buffer[4096];

    // Listen loop
    do {
        bytes_received = recv(sock_client, buffer, data_size, 0);

        pthread_mutex_lock(&(net_vars->lock)); // LOCK
        if (net_vars->show_data)
            printf("Bytes received: %ld\n", bytes_received);
        memcpy(data, buffer, data_size);
        pthread_mutex_unlock(&(net_vars->lock)); // UNLOCK
    } while (bytes_received > 0);

    pthread_mutex_lock(&(net_vars->lock)); // LOCK
    net_vars->active_clients[current_client] = false;
    pthread_mutex_unlock(&(net_vars->lock)); // UNLOCK
    pthread_exit(NULL);
}

// Allocate memory for network variables
void create_network(network_vars **net_vars, size_t max_clients) {
    (*net_vars) = calloc(1, sizeof(network_vars));
    (*net_vars)->clients_sockets = calloc(max_clients, sizeof(int));
    (*net_vars)->active_clients = calloc(max_clients, sizeof(bool));
    (*net_vars)->max_clients = max_clients;
    return;
}

// Init network variables
int init_network(network_vars *net_vars, pthread_t *threads, pthread_attr_t *attr) {
    net_vars->addr.sin_family = AF_INET;
    net_vars->addr.sin_port = htons(PORT);
    net_vars->addr.sin_addr.s_addr = INADDR_ANY;
    pthread_mutex_init(&(net_vars->lock), NULL); 
    for (size_t i = 0; i < net_vars->max_clients; i++)
        net_vars->active_clients[i] = false;

    net_vars->threads = threads;
    net_vars->attr = attr;
    net_vars->amount_clients = 0;
    net_vars->show_data = false;

    // Init socket
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
    free((*net_vars)->active_clients);
    free(*net_vars);
    *net_vars = NULL;
    return;
}

int accept_connection(network_vars *net_vars, size_t client_id) {
    if ((net_vars->clients_sockets[client_id] = accept(net_vars->server_socket, 0, 0)) == -1)
        return ERROR_ACCEPT;
    pthread_mutex_lock(&(net_vars->lock));
    net_vars->current_client = client_id;
    net_vars->active_clients[client_id] = true;
    net_vars->amount_clients++;
    pthread_mutex_unlock(&(net_vars->lock));
    return OK;
}

void stop_connecting(network_vars *net_vars) {
    shutdown(net_vars->server_socket, SHUT_RDWR);
    return;
}

void *wait_for_connections(void *args) {
    network_vars *net_vars = args;
    size_t client_id = 0;
    size_t max_clients = net_vars->max_clients;

    while (client_id < max_clients) { 
        printf("Waiting\n");
        if (check_error(accept_connection(net_vars, client_id)))
            break;
        printf("Connected client %ld\n", client_id);
        show_active_clients(net_vars);
        pthread_mutex_lock(&net_vars->lock); // LOCK
        pthread_create(&net_vars->threads[2*client_id], net_vars->attr, sendmessage, net_vars);
        pthread_create(&net_vars->threads[2*client_id + 1], net_vars->attr, listener, net_vars);
        pthread_mutex_unlock(&net_vars->lock); // UNLOCK
        client_id++;
    }
    pthread_exit(NULL);
}

size_t get_amount_clients(network_vars *net_vars) {
    size_t amount;
    pthread_mutex_lock(&net_vars->lock);
    amount =  net_vars->amount_clients;
    pthread_mutex_unlock(&net_vars->lock);
    return amount;
}

void show_active_clients(network_vars *net_vars) {
    pthread_mutex_lock(&net_vars->lock);
    for (size_t i = 0; i < net_vars->max_clients; i++)
        printf("CLIENT #%ld: %s\n", i, net_vars->active_clients[i] ? "CONNECTED" : "NOT CONNECTED");
    pthread_mutex_unlock(&net_vars->lock);
    return;
}

void show_data_in_out(network_vars *net_vars) {
    pthread_mutex_lock(&net_vars->lock);
    net_vars->show_data = !net_vars->show_data;
    printf("SHOW DATA: %s\n", net_vars->show_data ? "ON" : "OFF");
    pthread_mutex_unlock(&net_vars->lock);
    return; 
}

void disconnect_all_clients(network_vars *net_vars) {
    pthread_mutex_lock(&net_vars->lock);
    for (size_t i = 0; i < net_vars->max_clients; i++) {
        if (net_vars->active_clients[i]) {
            shutdown(net_vars->clients_sockets[i], SHUT_RDWR);
            net_vars->active_clients[i] = false;
        }
    }
    pthread_mutex_unlock(&net_vars->lock);
    return; 
}

int check_error(int error) {
    if (error == OK)
        return 0;
    fprintf(stderr, "FAIL ON ");
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
            fprintf(stderr, "ACCEPT or SERVER ENDED\n");
            break;
        case ERROR_CONNECT:
            fprintf(stderr, "CONNECT\n");
            break;
    }
    return 1;
}
