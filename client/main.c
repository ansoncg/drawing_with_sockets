#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "network.h"
#include "media.h"

#define MAX_CLIENTS 1

int main(void) {
    media *m = NULL;
    network_vars *net_vars = NULL;
    pthread_t threads[2];
    pthread_attr_t attr;
    pthread_mutex_t lock;
    pthread_mutex_init(&lock, NULL);

    // Create SDL, sockets and threads
    create_media(&m, MAX_CLIENTS);
    create_network(&net_vars, MAX_CLIENTS);
    init_network(net_vars, &lock);
    set_client_data(net_vars, get_data(m), get_data_size());
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    check_error(connect_to_server(net_vars));
    pthread_create(&threads[0], &attr, sendmessage, net_vars);
    pthread_create(&threads[1], &attr, listener, net_vars);

    while (is_running(m)) {
        mark_loop_beginning(m);

        pthread_mutex_lock(&lock);
        printf("SDL loop start\n");
        //get_input(m);
        draw(m, get_server_data(net_vars));
        printf("SDL loop end\n");
        pthread_mutex_unlock(&lock);
        mark_loop_duration(m);

        delay_loop(m);
    }

    destroy_media(&m);
    destroy_network(&net_vars);
    return EXIT_SUCCESS;
}
