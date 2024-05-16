#ifndef RC_FILTER
#define RC_FILTER

typedef struct {

    float a[3];
    float b[3];
    float in[3];
    float out[3];

}   RC_Filter;

void RC_Filter_Init(RC_Filter *filt, float cutoffFreqHz, float sampleTimeS);
float RC_Filter_Update(RC_Filter *filt, float inp);


#endif