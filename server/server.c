#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "media.h"
#include "network.h"

//TODO erros
#define MAX_CLIENTS 1

int main(void) {
    //int error;
    size_t client_id = 0;
    void *clients_data = NULL; // Clients application varibles 
    network_vars *net_vars = NULL; // Network information
    pthread_t threads[MAX_CLIENTS*2];
    pthread_attr_t attr;
    //pthread_key_t thread_key;

    // Create and init application varibles for all clients
    server_create_app_vars(&clients_data, MAX_CLIENTS);

    // Create and init network varibles
    create_network(&net_vars, MAX_CLIENTS); 
    check_error(init_network(net_vars));
    set_clients_data(net_vars, clients_data);

    // Create and init threads
    pthread_attr_init(&attr);
    //pthread_key_create(&thread_key, NULL);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    while (1) { //TODO criar condição de termino
        printf("Waiting\n");
        check_error(accept_connection(net_vars, client_id));
        printf("Connected client %ld\n", client_id);
        pthread_create(&threads[client_id], &attr, sendmessage, net_vars);
        pthread_create(&threads[client_id + 1], &attr, listener, net_vars);
        client_id++;
    }

    // Destroy application varibles
    server_destroy_app_vars(&clients_data);

    // Destroy network


    // Destroy threads

    return EXIT_SUCCESS;
}
