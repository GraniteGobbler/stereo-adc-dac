#include "IFX_PeakingFilter.h"

#define M_PI    3.14159265359   // My pi

void IFX_PeakingFilter_Init(IFX_PeakingFilter *filt, float sampleRate_Hz){

    /* Compute sample time */
    filt->sampleTime_s = 1.0f / sampleRate_Hz;    // Same as ADC

    /* Clear filter memory */
    for (uint8_t n = 0; n < 3; n++){

        filt->x[n] = 0.0f;
        filt->y[n] = 0.0f;

    }

    /* Calculate default (all-pass) filter coefficients */
    IFX_PeakingFilter_SetParameters(filt, 1.0f, 0.0f, 1.0f);

}

/* Compute filter coefficients, If boostCut_linear > 1 → boost, < 1 → cut */
void IFX_PeakingFilter_SetParameters(IFX_PeakingFilter *filt, float centerFrequency_Hz, float bandwidth_Hz, float boostCut_linear){

    /* Convert Hz to rad/s, pre-warp, multiply by sampling time (wc*T = 2 * tan(wc*T/2)) */
    float wcT = 2.0f * tanf(M_PI * centerFrequency_Hz * filt->sampleTime_s);

    /* Compute quality factor (Q = B / fc) */
    // float Q = bandwidth_Hz / centerFrequency_Hz;    // ! NOT PREWARPED ! 
    float Q = centerFrequency_Hz / bandwidth_Hz;    // ! NOT PREWARPED ! 

    /* Compute filter coefficients */
    filt->a[0] = 4.0f + 2.0f * (boostCut_linear / Q) * wcT + wcT * wcT;
    filt->a[1] = 2.0f * wcT * wcT - 8.0f;
    filt->a[2] = 4.0f - 2.0f * (boostCut_linear / Q) * wcT + wcT * wcT;

    filt->b[0] = 1.0f / (4.0f + 2.0f * (1.0f / Q) * wcT + wcT * wcT);    // Note: 1 / b0
    filt->b[1] = -(2.0f * wcT * wcT - 8.0f);                    // Note: -b1
    filt->b[2] = -(4.0f - 2.0f * (1.0f / Q) * wcT + wcT * wcT);          // Note: -b2
}


/* 2nd order peaking filter, 3 sample input, 3 sampe output. Two samples would suffice. */
float IFX_PeakingFilter_Update(IFX_PeakingFilter *filt, float in){

    /* Shift samples */
    filt->x[2] = filt->x[1];
    filt->x[1] = filt->x[0];
    filt->x[0] = in;
    
    filt->y[2] = filt->y[1];
    filt->y[1] = filt->y[0];

    /* Compute new filter output */
    filt->y[0] = (filt->a[0] * filt->x[0] + filt->a[1] * filt->x[1] + filt->a[2] * filt->x[2]
               +                           (filt->b[1] * filt->y[1] + filt->b[2] * filt->y[2])) * filt->b[0];
    
    /* Return current output sample */
    return (filt->y[0]);
    // return (in);
    
}
