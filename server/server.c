#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "media.h"
#include "network.h"

#define MAX_CLIENTS 3


int main(void) {
    void *clients_data = NULL; // Clients application variables 
    network_vars *net_vars = NULL; // Network information
    pthread_attr_t attr;
    pthread_t connection;
    pthread_t clients_threads[MAX_CLIENTS*2];
    char command[500];

    // Create and init application variables for all clients
    server_create_app_vars(&clients_data, MAX_CLIENTS);

    // Init attr and connection thread
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Create and init network variables
    create_network(&net_vars, MAX_CLIENTS); 
    check_error(init_network(net_vars, clients_threads, &attr));
    set_clients_data(net_vars, clients_data);

    // Start waiting for connections
    pthread_create(&connection, &attr, wait_for_connections, net_vars);

    // Read commands until exit
    scanf(" %s", command);
    while (strcmp(command, "exit")) {
        scanf(" %s", command);

    }

    // Destroy threads
    for (size_t i = 0; i < get_amount_clients(net_vars)*2; i++)
        pthread_join(clients_threads[i], NULL);

    // Destroy application variables
    server_destroy_app_vars(&clients_data);

    // Destroy network


    return EXIT_SUCCESS;
}
