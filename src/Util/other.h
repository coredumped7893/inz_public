//
// Created by Maciek Malik
//

#ifndef MM_other_H_
#define MM_other_H_

#ifdef __cplusplus
extern "C"{
#endif


int max(int a,int b);
int min(int a,int b);


#define CMP_DELTA_BIG 0.1
#define CMP_DELTA_SMALL 0.0001
#define CMP_DELTA_HEADING 2.0

int fp_compare(double f1, double  f2, double  epsilon);

#ifdef __cplusplus
}
#endif

#endif //MM_other_H_
