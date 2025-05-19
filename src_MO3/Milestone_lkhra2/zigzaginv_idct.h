#ifndef ZIGZAGINV_IDCT_H    // Garde d'inclusion : évite les inclusions multiples
#define ZIGZAGINV_IDCT_H

#include <stdint.h>
#include "decodage.h"
#define PI 3.14159265358979323846




void zigzag_inv(const int16_t in[64], int16_t out[8][8]);
void scalar_idct(const int16_t in[8][8], uint8_t out[8][8]);
uint8_t ****blocks_post_zigzaginv_idct(int16_t          ***deq,    // [c][blk][64] 
                               const ComponentInfo comp[],
                               int                 mcu_w,
                               int                 mcu_h,
                               uint8_t             N);



#endif
