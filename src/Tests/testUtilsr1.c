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

#include "../Util/other.h"

void test_util_max(void** state){
    UNUSED(state);

    int a = 5;
    int b = -1;

    assert_int_equal(max(a,b),a);

    assert_int_equal(max(a,b),max(b,a));

    assert_int_equal(max(a,a),a);

}

void test_util_min(void** state){
    UNUSED(state);

    int a = 5;
    int b = -1;

    assert_int_equal(min(a,b),b);

    assert_int_equal(min(a,b),min(b,a));

    assert_int_equal(min(a,a),a);

}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_util_max),
        cmocka_unit_test(test_util_min),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
