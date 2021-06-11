#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "network.h"
#include "media.h"

#define MAX_CLIENTS 3

int main(void) {
    media *m = NULL;
    network_vars *net_vars = NULL;
    pthread_t threads[2];
    pthread_attr_t attr;
    pthread_mutex_t lock;
    pthread_cond_t cond;

    // Create and init SDL
    create_media(&m, MAX_CLIENTS);
    init_media(m);

    // Create and init sockets
    create_network(&net_vars, MAX_CLIENTS);
    init_network(net_vars, &lock, &cond);

    // Insert the memory spaces being used by the media on the network module
    set_app_data(net_vars, get_local_data(m), get_server_data(m), get_data_size());

    // Init mutex and cond 
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    check_error(connect_to_server(net_vars));
    pthread_create(&threads[0], &attr, sendmessage, net_vars);
    pthread_create(&threads[1], &attr, listener, net_vars);

    // Main loop
    while (is_running(m)) {
        mark_loop_beginning(m);

        pthread_mutex_lock(&lock); // LOCK
        get_input(m);
        draw(m);
        pthread_mutex_unlock(&lock); // UNLOCK

        pthread_cond_signal(&cond); // SIGNAL
        mark_loop_duration(m);
        delay_loop(m);
    }

    destroy_media(&m);
    destroy_network(&net_vars);
    return EXIT_SUCCESS;
}
