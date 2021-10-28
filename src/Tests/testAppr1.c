//
// Created by Maciek Malik
//
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>
#include <stdlib.h>

#define UNUSED(x) (void)(x)

#include "../FlightManager/app.h"
#include "../DataInput/xPlane.h"

void throw_err(uint8_t id, uint8_t exitOnErr){
    //Disable throwing errors in those tests
    return;
}


void* __real_calloc (size_t num, size_t size);
void* __wrap_calloc (size_t num, size_t size){
    check_expected(size);
    return (void*)mock();
}

int __real_destroy_inet_socket(int fd);
int __wrap_destroy_inet_socket(int fd){
    check_expected(fd);
    return (int)mock();
}

void __real_exit(int status);
void __wrap_exit(int status){
    return;
}

int __real_printf(const char* format, ...);
int __wrap_printf(const char* format, ...){
    return 0;
}

void __real_perror(const char *s);
void __wrap_perror(const char *s){
    return;
}


int x_plane_socket;

void test_alloc_frame(void** state){
    UNUSED(state);

    expect_value(__wrap_calloc,size,sizeof(status_frame));
    will_return(__wrap_calloc,(void*)__real_calloc(1,sizeof(status_frame)));

    status_frame* frame = NULL;
    frame = alloc_frame();
    assert_non_null(frame);

}





//        printf("EPBC->WP1: %f\n", get_target_heading(52.269f,20.908f,52.284f,20.989f));
//        printf("WP2->WP3: %f\n", get_target_heading(52.26f,21.007f,52.255f,20.982f));
//        printf("WP3->WP2: %f\n", get_target_heading(52.255f,20.982f,52.26f,21.007f));
//        printf("WP3->WP4: %f\n", get_target_heading(52.255f,20.982f,52.244f,21.001f));
//        return NULL;








void test_alloc_frame_calloc(void** state){
    UNUSED(state);

    expect_value(__wrap_calloc,size,sizeof(status_frame));
    will_return(__wrap_calloc,NULL);

    status_frame* frame = NULL;
    frame = alloc_frame();
    assert_null(frame);

}

void test_cleanUp(void** state){
    UNUSED(state);


    x_plane_socket = -5;

    expect_value(__wrap_destroy_inet_socket,fd,-5);
    will_return(__wrap_destroy_inet_socket,-1);

    //Exec testing func
    clean_up();


    x_plane_socket = 48;

    expect_value(__wrap_destroy_inet_socket,fd,48);
    will_return(__wrap_destroy_inet_socket,0);

    //Exec testing func
    clean_up();

}

void test_alloc_socket_packet(void** state){
    UNUSED(state);

    expect_value(__wrap_calloc,size,sizeof(socket_packet_t));
    will_return(__wrap_calloc,(void*)__real_calloc(1,sizeof(socket_packet_t)));

    socket_packet_t* sp = alloc_socket_packet();
    assert_non_null(sp);


    expect_value(__wrap_calloc,size,sizeof(socket_packet_t));
    will_return(__wrap_calloc,NULL);

    sp = alloc_socket_packet();
    assert_null(sp);


}


void test_printConfig(void** state){
    UNUSED(state);
    printConfig();
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_alloc_frame),
        cmocka_unit_test(test_alloc_frame_calloc),
        cmocka_unit_test(test_cleanUp),
        cmocka_unit_test(test_printConfig),
        cmocka_unit_test(test_alloc_socket_packet),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
