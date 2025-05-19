#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "zigzaginv_idct.h"


void zigzag_inv(const int16_t in[64], int16_t out[8][8])
{
    static const uint8_t zz[64] = {
         0,  1,  5,  6, 14, 15, 27, 28,
         2,  4,  7, 13, 16, 26, 29, 42,
         3,  8, 12, 17, 25, 30, 41, 43,
         9, 11, 18, 24, 31, 40, 44, 53,
        10, 19, 23, 32, 39, 45, 52, 54,
        20, 22, 33, 38, 46, 51, 55, 60,
        21, 34, 37, 47, 50, 56, 59, 61,
        35, 36, 48, 49, 57, 58, 62, 63
    };
    for (uint8_t i = 0; i < 8; ++i)
        for (uint8_t j = 0; j < 8; ++j)
            out[i][j] = in[ zz[i*8 + j] ];
}


void scalar_idct(const int16_t in[8][8], uint8_t out[8][8])
{
    const double c[8] = { 0.70710678,1,1,1,1,1,1,1 };
    for (uint8_t x = 0; x < 8; ++x)
        for (uint8_t y = 0; y < 8; ++y) {
            double s = 0.0;
            for (uint8_t u = 0; u < 8; ++u)
                for (uint8_t v = 0; v < 8; ++v)
                    s += c[u]*c[v]*in[u][v] *cos((2*x+1)*u*PI/16.0) *cos((2*y+1)*v*PI/16.0);
            int v = (int)lrint(s/4.0) + 128;
            out[x][y] = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
        }
}


uint8_t ****blocks_post_zigzaginv_idct(int16_t          ***deq,    // [c][blk][64] 
                               const ComponentInfo comp[],
                               int                 mcu_w,
                               int                 mcu_h,
                               uint8_t             N)
{
    uint8_t ****final = malloc(N * sizeof *final);  //[c][8][8]

    for (uint8_t c = 0; c < N; ++c) {

        const int nb_blocks = mcu_w * mcu_h *comp[c].h_samp * comp[c].v_samp;

        final[c] = malloc(nb_blocks * sizeof **final);

        for (int b = 0; b < nb_blocks; ++b) {

            final[c][b] = malloc(8 * sizeof ***final);

            int16_t post_zigzag_inv[8][8];
            uint8_t post_idct [8][8];

            zigzag_inv(deq[c][b], post_zigzag_inv);
            scalar_idct(post_zigzag_inv, post_idct);

            for (uint8_t r = 0; r < 8; ++r) {
                final[c][b][r] = malloc(8);
                memcpy(final[c][b][r], post_idct[r], 8);
            }
        }
    }
    return final;
}


void free_blocks_zigzaginv_idct(int16_t*** blocks,    // [c][blk][64] 
    const ComponentInfo comp[], int mcu_w, int mcu_h, uint8_t N){

    for (int c=0; c<N; c++){
        const int nb_blocks = mcu_w * mcu_h *comp[c].h_samp * comp[c].v_samp;
        for (int b=0; b<nb_blocks; b++){
            for ( int r=0; r<8; r++){
                free(blocks[c][b][r]);
            }
            free(blocks[c][b]);
        }
        free(blocks[c]);
    }
    free(blocks);
}
