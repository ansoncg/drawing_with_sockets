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
    bool locked;
};

void locknet(network_vars *net_vars) {
    net_vars->locked = true; 
    return;
}

void unlocknet(network_vars *network_vars) {
    network_vars->locked = false;
    return;
}

void *get_server_data(network_vars *net_vars) {
    return net_vars->server_data;
}

void *sendmessage(void *args) {
    network_vars *net_vars = (network_vars *) args;
    ssize_t bytes_sent;
    size_t buffer_size;
    char buffer[4096];

    do {
        pthread_mutex_lock((net_vars->lock));
        buffer_size = serialize(buffer, net_vars->client_data, 1);
        bytes_sent = send(net_vars->socket, buffer, buffer_size, 0);
        printf("bytes send %ld\n", bytes_sent);
        pthread_mutex_unlock((net_vars->lock));
    } while (1); // TODO

    //close(my_socket);
    pthread_exit(NULL);
}

void *listener(void *args) {
    network_vars *net_vars = (network_vars *) args;
    ssize_t bytes_received;
    size_t buffer_size;
    //size_t data_size = net_vars->max_clients * net_vars->data_size;
    char buffer[4096];

    buffer_size = serialize(buffer, net_vars->server_data, net_vars->max_clients); 
    do {
        pthread_mutex_lock((net_vars->lock));
        bytes_received = recv(net_vars->socket, buffer, buffer_size, 0);
        deserialize(buffer, net_vars->server_data, net_vars->max_clients);
        printf("bytes received %ld\n", bytes_received);
        pthread_mutex_unlock((net_vars->lock));
    } while (1);
    pthread_exit(NULL);
}

void set_client_data(network_vars *net_vars, void *clients_data, size_t data_size) {
    net_vars->client_data = clients_data;
    net_vars->data_size = data_size;
    net_vars->server_data = calloc(1, data_size*net_vars->max_clients);
    return;
}

void create_network(network_vars **net_vars, size_t max_clients) {
    (*net_vars) = calloc(1, sizeof(network_vars));
    (*net_vars)->max_clients = max_clients;
    return;
}

int init_network(network_vars *net_vars, pthread_mutex_t *lock) {
    net_vars->addr.sin_family = AF_INET;
    net_vars->addr.sin_port = htons(PORT);
    net_vars->addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    net_vars->lock = lock;
    if ((net_vars->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return ERROR_SOCKET;
    return OK; 
}

void destroy_network(network_vars **net_vars) {
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
    exit(1);
    return;
}

int connect_to_server(network_vars *net_vars) {
    if (connect(net_vars->socket, (struct sockaddr *) &(net_vars->addr), sizeof(net_vars->addr)) == -1) 
        return ERROR_CONNECT;
    printf("Connected to server\n");
    return OK;
}

/*
   int main(void) {
   int my_socket; 
   struct sockaddr_in addr = {
   .sin_family = AF_INET,
   .sin_port = htons(PORT),
   .sin_addr.s_addr = inet_addr("127.0.0.1")
   }; // Struct initialization initialize the rest with 0.
   pthread_mutex_t mutexsum = PTHREAD_MUTEX_INITIALIZER;
   pthread_t threads[2];
   pthread_attr_t attr;

   if ((my_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
   fprintf(stderr, "Error on socket - client\n");
   return EXIT_FAILURE;
   }

   printf("Trying to connect...\n");

   if (connect(my_socket, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
   fprintf(stderr, "Error on connect.\n");
   return EXIT_FAILURE;
   }

   printf("Connected.\n");

   sendmessage_args s_args = {.my_socket = my_socket, .mutexsum = &mutexsum};
   listener_args l_args = {.my_socket = my_socket};

// Threads
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
pthread_create(&threads[0], &attr, sendmessage, &s_args);
pthread_create(&threads[1], &attr, listener, &l_args);

//while (running) { 

//}
pthread_exit(NULL);

//close(my_socket); 
return EXIT_SUCCESS;
}
*/
