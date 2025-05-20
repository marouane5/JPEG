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
            idct_rapide(post_zigzag_inv, post_idct);

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
            free(blocks[c][b]);
        }
        free(blocks[c]);
    }
    free(blocks);
}

float* papillon(float x, float y) {
    float* res = malloc(2 * sizeof(float));
    res[0] = (x + y) / 2;
    res[1] = (x - y) / 2;
    return res;
}

// Même principe ici pour rotation
float* rotation(float x, float y, float k, uint8_t n) {
    float* res = malloc(2 * sizeof(float));
    res[0] = (x * cos((n*PI) / 16) - y * sin((n*PI) / 16))/ k;
    res[1] = (y * cos((n*PI) / 16) + x * sin((n*PI) / 16))/ k;
    return res;
}

float* manip_vector_idct(float* stage_4) {
    float stage3[8]; float stage_2[8];float stage_1[8];float stage_0[8];

    for(uint8_t i = 0; i < 8; i++) {
        stage_4[i] = sqrt(8) * stage_4[i];
    }

    // Étape 1 faite en parallèle (possibilité mentionnée dans l'article de référence )
    stage3[0] = stage_4[0]; stage3[6] = stage_4[6]; stage3[4] = stage_4[4]; stage3[2] = stage_4[2];

    float* tmp = papillon(stage_4[1], stage_4[7]); stage3[1] = tmp[0]; stage3[7] = tmp[1];
    free(tmp);

    stage3[3] = stage_4[3] / sqrt(2);
    stage3[5] = stage_4[5] / sqrt(2);

    /* On passe à l'Étape 2 */
    tmp = papillon(stage3[0], stage3[4]);
    stage_2[0] = tmp[0]; stage_2[4] = tmp[1];
    free(tmp);

    tmp = rotation(stage3[2], stage3[6], sqrt(2), 6);
    stage_2[2] = tmp[0]; stage_2[6] = tmp[1];
    free(tmp);

    tmp = papillon(stage3[7], stage3[5]);
    stage_2[7] = tmp[0]; stage_2[5] = tmp[1];
    free(tmp);

    tmp = papillon(stage3[1], stage3[3]);
    stage_2[1] = tmp[0]; stage_2[3] = tmp[1];
    free(tmp);

    /* Étape 3 */
    tmp = papillon(stage_2[0], stage_2[6]); 
    stage_1[0] = tmp[0]; stage_1[6] = tmp[1];
    free(tmp);

    tmp = papillon(stage_2[4], stage_2[2]);
    stage_1[4] = tmp[0]; stage_1[2] = tmp[1];
    free(tmp);

    tmp = rotation(stage_2[7], stage_2[1], 1, 3);
    stage_1[7] = tmp[0]; stage_1[1] = tmp[1];
    free(tmp);

    tmp = rotation(stage_2[3], stage_2[5], 1, 1);
    stage_1[3] = tmp[0]; stage_1[5] = tmp[1];
    free(tmp);

    /* Étape 4 */
    tmp = papillon(stage_1[0], stage_1[1]);
    stage_0[0] = tmp[0]; stage_0[1] = tmp[1];
    free(tmp);

    tmp = papillon(stage_1[4], stage_1[5]);
    stage_0[4] = tmp[0]; stage_0[5] = tmp[1];
    free(tmp);

    tmp = papillon(stage_1[2], stage_1[3]);
    stage_0[2] = tmp[0]; stage_0[3] = tmp[1];
    free(tmp);

    tmp = papillon(stage_1[6], stage_1[7]);
    stage_0[6] = tmp[0]; stage_0[7] = tmp[1];
    free(tmp);

    /* On retrouve le résultat via une permulation */
    float* res = malloc(8 * sizeof(float));
    res[0] = stage_0[0];
    res[1] = stage_0[4];
    res[2] = stage_0[2];
    res[3] = stage_0[6];
    res[4] = stage_0[7];
    res[5] = stage_0[3];
    res[6] = stage_0[5];
    res[7] = stage_0[1];

    return res;
}

void idct_rapide(int16_t post_zigzag_inv[8][8], uint8_t post_idct[8][8]) {
    float tmp[8][8];

    for(uint8_t j = 0; j < 8; j++) {
        float colonne[8];
        for(uint8_t i = 0; i < 8; i++) {
            colonne[i] = post_zigzag_inv[i][j];
        }


        float* colonne_post_manip = manip_vector_idct(colonne);
        /* On recopie les resultats de la fonction manip dans tmp pour l'utiliser */
        for(uint8_t i = 0; i < 8; i++) {
            tmp[i][j] = colonne_post_manip[i];
        }
        
        free(colonne_post_manip);
    }

    for(uint8_t i = 0; i < 8; i++) {
        float ligne[8];
        
        for(uint8_t j = 0; j < 8; j++) {
            ligne[j] = tmp[i][j];
        }
        float* ligne_post_manip = manip_vector_idct(ligne);
        
        for(uint8_t j = 0; j < 8; j++) {
            float valeur_tmp = ligne_post_manip[j];
            valeur_tmp += 128;
            
            /* Etape post idct (Saturation)*/
            if(valeur_tmp < 0) valeur_tmp = 0;
            if(valeur_tmp > 255) valeur_tmp = 255;
            post_idct[i][j] = (uint8_t) valeur_tmp;
        }
        
        free(ligne_post_manip);
    }
}

/* Ancienne version IDCT (lente) */
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
