#include "ParamEQ.h"

#define M_PI    3.14159265359   // My pi

void ParamEQ_Init(ParamEQ *filt, float fs_Hz){

    /* Compute sample time */
    filt->T_s = 1.0f / fs_Hz;    // Same as ADC

    /* Clear filter memory */
    for (uint8_t n = 0; n < 3; n++){

        filt->x[n] = 0.0f;
        filt->y[n] = 0.0f;

    }

    /* Calculate default (all-pass) filter coefficients */
    ParamEQ_SetParameters(filt, 1.0f, 0.0f, 1.0f);

}

/* Compute filter coefficients, If g > 1 → boost, < 1 → cut */
void ParamEQ_SetParameters(ParamEQ *filt, float fc_Hz, float B_Hz, float g){

    /* Convert Hz to rad/s, pre-warp, multiply by sampling time (wc*T = 2 * tan(wc*T/2)) */
    float wcT = 2.0f * tanf(M_PI * fc_Hz * filt->T_s);

    /* Q = fc / B */
    float Q = fc_Hz / B_Hz;    // ! NOT PREWARPED ! 

    /* Compute filter coefficients */
    filt->b[0] = 4.0f + 2.0f * (g / Q) * wcT + wcT * wcT;
    filt->b[1] = 2.0f * wcT * wcT - 8.0f;
    filt->b[2] = 4.0f - 2.0f * (g / Q) * wcT + wcT * wcT;

    filt->a[0] = 1.0f / (4.0f + 2.0f * (1.0f / Q) * wcT + wcT * wcT);   // Note: 1 / a0
    filt->a[1] = -(2.0f * wcT * wcT - 8.0f);                            // Note: -a1
    filt->a[2] = -(4.0f - 2.0f * (1.0f / Q) * wcT + wcT * wcT);         // Note: -a2
}


/* 2nd order peaking filter, 3 sample input, 3 sampe output. Two samples would suffice. */
float ParamEQ_Update(ParamEQ *filt, float in){

    /* Shift samples */
    filt->x[2] = filt->x[1];
    filt->x[1] = filt->x[0];
    filt->x[0] = in;
    
    filt->y[2] = filt->y[1];
    filt->y[1] = filt->y[0];

    /* Compute new filter output */
    filt->y[0] = filt->a[0] * (filt->b[0] * filt->x[0] + filt->b[1] * filt->x[1] + filt->b[2] * filt->x[2]
               + filt->a[1] * filt->y[1] + filt->a[2] * filt->y[2]);
    
    /* Return current output sample */
    return (filt->y[0]);

}
