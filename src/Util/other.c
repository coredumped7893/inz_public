//
// Created by Maciek Malik
//

#include "other.h"


/**
 *
 * @param a
 * @param b
 * @return Returns bigger value from two int params
 */
int max(int a, int b) {
    if(a>=b) return a;
    return b;
}

/**
 * @param a
 * @param b
 * @return Returns smaller value from two int params
 */
int min(int a, int b) {
    if(a<=b) return a;
    return b;
}

/**
 * Floating Point number compare with custom epsilon value
 * @param f1
 * @param f2
 * @param epsilon
 * @return
 */
int fp_compare(double  f1, double  f2, double  epsilon) {

    if( (f2-epsilon) < f1 && f1 < (f2+epsilon)){
        return 1;
    }else{
        return 0;
    }

}
