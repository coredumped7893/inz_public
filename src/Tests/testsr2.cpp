//
// Created by Maciek Malik
//
#include <locale>

#ifdef __cplusplus
extern "C" {
#endif


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>

#include <cmocka.h>


//required for tests
#include "../ImageProcessing/wgs84_do_puwg92.h"
//------------------



#define UNUSED(x) (void)(x)


//int __real_printf(const char *__restrict format, ...);
//int __wrap_printf(const char *__restrict format, ...){
//
//    //Get param list
////    va_list args;
////    va_start(args, format);
////    double first_arg = va_arg(args,double);
////    double second_arg = va_arg(args,double);
////    va_end(args);
//
//    //Check function params in the test
//    check_expected_ptr(format);
////    check_expected(first_arg);
////    check_expected(second_arg);
//
//    return (int)mock();
//}

//extern void test_wgs_pl1992_convert();
//
//void test_wgs_to_pl92_text_dump(void** state){
//    test_wgs_pl1992_convert();
//    assert_true(true);
//}

void test_wgs_to_pl92(void** state){
    UNUSED(state);

    double B_stopnie=48.00;
    double L_stopnie=13.00;
    double Xpuwg;
    double Ypuwg;

/*
    WGS 84 -> PUWG 1992
    B:    48.0000  L:    13.0000
    X: 32141.9847  Y: 52647.1101
 */
    int ret = wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg);
    assert_int_equal(ret,0);
    assert_in_range(Xpuwg,32141,32142);
    assert_in_range(Ypuwg,52647,52648);


}

//void test_convert_printf(void** state){
//    UNUSED(state);
//
//    //What we want the function to do/to return
//    expect_string(__wrap_printf,format,"\nErr: invalid input params.\n");
//    will_return(__wrap_printf,28);
//
//    //execute it
//    int ret = printf("\nErr: invalid input params.\n");
//
//    //Check what it returned
//    assert_int_equal(ret,28);
//
//}

void test_pl92_to_wgs(void** state){
    UNUSED(state);

    double Xpuwg = 32141.985;
    double Ypuwg = 52647.11;
    double B1_stopnie;
    double L1_stopnie;

/*
    PUWG 1992 -> WGS 84
    X: 32141.9847  Y: 52647.1101
    B:    48.0000  L:    13.0000
 */

    int ret = puwg92_do_wgs84(Xpuwg, Ypuwg, &B1_stopnie, &L1_stopnie);
    assert_int_equal(ret,0);
    assert_float_equal(B1_stopnie,48,0.01);
    assert_float_equal(L1_stopnie,13,0.01);

}


void test_wgs_pl92_arg_failB(void** state){
    UNUSED(state);

    double B_stopnie=78.00;
    double L_stopnie=13.00;
    double Xpuwg=0;
    double Ypuwg=0;

    int ret = wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg);
    assert_int_equal(ret,1);
    assert_int_equal(Xpuwg,0);
    assert_int_equal(Ypuwg,0);


     B_stopnie=8.00;
     L_stopnie=13.00;
     Xpuwg=0;
     Ypuwg=0;

     ret = wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg);
    assert_int_equal(ret,1);
    assert_int_equal(Xpuwg,0);
    assert_int_equal(Ypuwg,0);

}

void test_wgs_pl92_arg_failL(void** state){
    UNUSED(state);

    double B_stopnie=48.00;
    double L_stopnie=73.00;
    double Xpuwg=0;
    double Ypuwg=0;

    int ret = wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg);
    assert_int_equal(ret,2);
    assert_int_equal(Xpuwg,0);
    assert_int_equal(Ypuwg,0);


     B_stopnie=48.00;
     L_stopnie=3.00;
     Xpuwg=0;
     Ypuwg=0;

     ret = wgs84_do_puwg92(B_stopnie, L_stopnie, &Xpuwg, &Ypuwg);
    assert_int_equal(ret,2);
    assert_int_equal(Xpuwg,0);
    assert_int_equal(Ypuwg,0);


}


int main(void){

    //Default stdout output is broken for g++
    //cmocka_set_message_output(CM_OUTPUT_XML);

    const struct CMUnitTest tests[] = {
            //cmocka_unit_test(test_wgs_to_pl92_text_dump),
            cmocka_unit_test(test_wgs_to_pl92),
            cmocka_unit_test(test_pl92_to_wgs),
            //cmocka_unit_test(test_convert_printf),
            cmocka_unit_test(test_wgs_pl92_arg_failB),
            cmocka_unit_test(test_wgs_pl92_arg_failL),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

#ifdef __cplusplus
}
#endif