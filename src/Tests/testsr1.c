//
// Created by Maciek Malik
//


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdio.h>

#define UNUSED(x) (void)(x)

#include "../Util/message.h"
#include "../Util/buff_tools.h"


int __real_printf (const char *format, ...);
int __wrap_printf (const char *format, ...){
    check_expected_ptr(format);


    char* arg_first = NULL;
    va_list args;
    va_start(args,format);
    arg_first = va_arg(args,char*);//Get string passed in through arg1
    va_end(args);

    check_expected_ptr(arg_first);//Force check argument value

    return 0;
}


void __real_throw_err(uint8_t id, uint8_t exitOnErr);
void __wrap_throw_err(uint8_t id, uint8_t exitOnErr) {
    check_expected(id);//Check id range
    check_expected(exitOnErr);
    //printf("------------------ Testing errors ------------------\n");
    throw_err(id,0);
    //printf("\n--------------------------------------------------\n");
}

void test_error_list_not_empty(void** state){
    UNUSED(state);
    //List is not empty
    assert_true((sizeof(ERR_LIST)/50) > 0);
}

//void print_message_local(uint8_t id) {
//    const char* str = MSG_LIST[id];
//    check_expected_ptr(str);
//}

void test_buff_append(void** state){
    UNUSED(state);

    uint8_t* str_appended = buff_append((uint8_t*)"Aaa",(uint8_t*)"Bbb",3,3);
    assert_string_equal(str_appended,"AaaBbb");

    str_appended = buff_append((uint8_t*)"",(uint8_t*)"",0,0);
    assert_string_equal(str_appended,"");

}

void test_float2Bytes(void** state){
    UNUSED(state);

    unsigned char bytes[4] = {0};
    float testVal = (float) 0.5;

    float2Bytes(bytes,testVal);

    uint8_t sample[4] = {0x00,0x00,0x00,0x3f};
    assert_memory_equal(bytes,sample,4);

    testVal = 0;
    float2Bytes(bytes,testVal);
    uint8_t sample2[4] = {0x00,0x00,0x00,0x00};
    assert_memory_equal(bytes,sample2,4);

    float2Bytes(NULL,testVal);
    assert_memory_equal(bytes,sample2,4);

}

void test_int2bytes(void** state){
    UNUSED(state);

    unsigned char bytes[4] = {0};
    int value = 0x0000aa55;

    int2bytes(bytes,value);
    uint8_t sample[4] = {0x55,0xaa,0x00,0x00};
    assert_memory_equal(bytes,sample,4);

    unsigned char bytes2[4] = {0};
    int2bytes(NULL,value);
    uint8_t sample2[4] = {0x00,0x00,0x00,0x00};
    assert_memory_equal(bytes2,sample2,4);

}

void test_print_message(void** state){
    UNUSED(state);
    expect_string(__wrap_printf,format," [%s] \n");
    expect_string(__wrap_printf,arg_first,"Init OK");
    print_message_local(0);

}

void test_throw_error(void** state){
    UNUSED(state);
    expect_in_range(__wrap_throw_err,id,0,sizeof(ERR_LIST)/50);
    expect_in_range(__wrap_throw_err,exitOnErr,0,1);
    expect_string(__wrap_printf,format,"\n ->[%s]<- \n");
    expect_string(__wrap_printf,arg_first,"Couldn't allocate memory");
    __wrap_throw_err(3,0);

    expect_in_range(__wrap_throw_err,id,0,sizeof(ERR_LIST)/50);
    expect_in_range(__wrap_throw_err,exitOnErr,0,1);
    expect_string(__wrap_printf,format,"\n ->[%s]<- \n");
    expect_string(__wrap_printf,arg_first,"Failed to initialize modules");
    __wrap_throw_err(0,0);
}


int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(test_throw_error),
            cmocka_unit_test(test_error_list_not_empty),
            cmocka_unit_test(test_print_message),
            cmocka_unit_test(test_buff_append),
            cmocka_unit_test(test_float2Bytes),
            cmocka_unit_test(test_int2bytes)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
