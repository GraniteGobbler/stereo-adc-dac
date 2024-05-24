#ifndef PARAM_EQ_H
#define PARAM_EQ_H

#include <math.h>
#include <stdint.h>


typedef struct{

    /* Sample time (s) */
    float T_s;

    /* Filter inputs */
    float x[3];

    /* Filter outputs */
    float y[3];

    /* y[n] coeffs */
    float a[3];

    /* x[n] coeffs */
    float b[3];

}   ParamEQ;

void ParamEQ_Init(ParamEQ *filt, float fs_Hz);
void ParamEQ_SetParameters(ParamEQ *filt, float fc_Hz, float B_Hz, float g);
float ParamEQ_Update(ParamEQ *filt, float in);


#endif