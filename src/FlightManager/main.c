//
// Created by Maciek Malik
//

#include "stdlib.h"
#include "stdio.h"
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "app.h"
#include "libinetsocket.h"
#include "libunixsocket.h"
#include "../Util/message.h"
#include "../Control/control.h"


/**
 * @deprecated
 * @return
 */
int* get_random() {
    int* t = calloc(1, sizeof(int));
    *t = rand() % 1000;
    return t;
}

/**
 * @test
 */
void test1(){
    //Test 1 -------------

    //set_throttle(0.9f);

    //send_command(1);//Flaps down
//
//        send_data(-0.92,8);
//        send_data(0.955,9);
//        send_data(0.55,10);

    AxisSet* axis = alloc_axis_set();
    axis->yaw = 0;
    axis->pitch = 0.6;
    axis->roll = 1;
    //set_axis(axis);
    free(axis);

    status_frame* f = get_status(NULL,0);

    free(f);

    //--------------------
}

/**
 * @brief Starting point for FlightManager module
 * @return
 */
TESTING_REQ_WEAK
int main(){

    //Start & init modules
    if(!startAll()){
        throw_err(0, 1);
    }else{
        printf("\n[Init OK]\n");
    }

    run();
    clean_up();

    return 0;
}

/**
 * @test
 * @deprecated
 */
void test_unix_socket(){
    int sfd, cfd, bytes, ret;
    char buf[16];
    int retW;
    buf[15] = 0;

    ret = sfd = create_unix_server_socket("/tmp/echosock", LIBSOCKET_STREAM, 0);

    if (ret < 0) {
        throw_err(2, 1);
    }

    print_message_local(1);

    for (;;) {
        ret = cfd = accept_unix_stream_socket(sfd, 0);

        if (ret < 0) {
            perror(0);
            exit(1);
        }

        while (0 < (bytes = read(cfd, buf, 15))) {
           retW = write(cfd, buf, bytes);
            retW = write(1, buf, bytes);
        }

        ret = destroy_unix_socket(cfd);

        if (ret < 0) {
            perror(0);
            exit(1);
        }
    }
    retW++;
    ret = destroy_unix_socket(sfd);

    if (ret < 0) {
        perror(0);
        exit(1);
    }
}

/**
 * @test
 * @deprecated
 */
void test_sockets(){
    int sfd, bytes, ret;
    char src_host[128], src_service[7], buf[16];

    src_host[127] = 0;
    src_service[6] = 0;

    sfd = create_inet_server_socket("0.0.0.0", "1234", LIBSOCKET_UDP,
                                    LIBSOCKET_IPv4, 0);

    if (-1 == sfd) {
        perror("couldn't create server\n");
        exit(1);
    }

    printf("Socket up and running\n");

    while (1) {
        memset(buf, 0, 16);
        ret = bytes = recvfrom_inet_dgram_socket(
                sfd, buf, 15, src_host, 127, src_service, 6, 0, LIBSOCKET_NUMERIC);

        if (ret < 0) {
            perror(0);
            exit(1);
        }

        ret =
                sendto_inet_dgram_socket(sfd, buf, bytes, src_host, src_service, 0);

        if (ret < 0) {
            perror(0);
            exit(1);
        }

        printf("Connection from %s port %s: %s (%i)\n", src_host, src_service,
               buf, bytes);
        printf("Connection processed\n");
    }

    ret = destroy_inet_socket(sfd);

    if (ret < 0) {
        perror(0);
        exit(1);
    }

}

