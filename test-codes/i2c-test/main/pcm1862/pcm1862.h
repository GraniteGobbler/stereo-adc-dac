#ifndef pcm1862
#define pcm1862

/*          Includes        */
#include "esp_log.h"
#include "driver/i2c.h"
#include <stdio.h>

/*          Defines         */
#define I2C_SLAVE_ADDR  0x4A
#define TIMEOUT_MS      1000
#define DELAY_MS        1000

/*          Prototypes      */

extern void pcm1862_init(void);




#endif