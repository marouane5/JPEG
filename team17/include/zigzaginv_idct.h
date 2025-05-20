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

void free_blocks_zigzaginv_idct(int16_t*** blocks,
    const ComponentInfo comp[], int mcu_w, int mcu_h, uint8_t N);

float* papillon(float x, float y);

float* rotation(float x, float y, float k, uint8_t n) ;

float* manip_vector_idct(float* stage_4);

void idct_rapide(int16_t post_zigzag_inv[8][8],uint8_t post_idct [8][8]);

void scalar_idct(const int16_t in[8][8], uint8_t out[8][8]);

#endif
