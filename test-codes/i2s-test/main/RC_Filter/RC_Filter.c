#include "RC_Filter.h"

void RC_Filter_Init(RC_Filter *filt, float cutoffFreqHz, float sampleFreqHz){

    /* Compute RC constant RC = 1/(2pi * fc) */
    float RC = 1.0f / (6.28318530718f * cutoffFreqHz);

    /* Convert sample frequency to sample time */
    float sampleTimeS = 1.0f / sampleFreqHz;

    /* Compute coefficients */
    // filt->coeff[0] = sampleTimeS / (sampleTimeS + RC);
    // filt->coeff[1] = RC / (sampleTimeS + RC);
    filt->a[0] = 4.5523;
    filt->a[1] = -7.1554;
    filt->a[2] = 4.2923;

    filt->b[0] = 4.6822;
    filt->b[1] = -7.1554;
    filt->b[2] = 4.1624;

    /* Clear output buffer */
    filt->in[0] = 0.0f;
    filt->in[1] = 0.0f;
    filt->in[2] = 0.0f;

    filt->out[0] = 0.0f;
    filt->out[1] = 0.0f;
    filt->out[2] = 0.0f;


}

float RC_Filter_Update(RC_Filter *filt, float inp){

    /* Shifting samples */
    // filt->outz = filt->out[0];
    // filt->out[1] = filt->out[0];
    filt->in[2] = filt->in[1];
    filt->in[1] = filt->in[0];
    filt->in[0] = inp;

    filt->out[2] = filt->out[1];
    filt->out[1] = filt->out[0];

    /* Compute new sample */
    // filt->out[0] = filt->coeff[0] * inp + filt->coeff[1] * filt->out[1];
    // filt->out[0] = 1.0f * inp;
    filt->out[0] = filt->b[0] * filt->in[0]
                    + filt->b[1] * filt->in[1]
                    + filt->b[2] * filt->in[2]
                    - filt->a[1] * filt->out[1]
                    - filt->a[2] * filt->out[2];

    
#if 0
    return (filt->out[0]);
#else
    return (filt->in[0]);   
#endif
}