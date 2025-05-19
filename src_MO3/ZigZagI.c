#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "quantifInv.c"
#define PI 3.14159265358979323846

/* Fonction ZigZagI pour le cas d'une liste contenant 64 élements */
int16_t**** ZigZagI(int16_t*** L,const ComponentInfo* comp, int total_mcu, uint16_t N){
    
    int16_t**** blocs_reorganises = malloc(N*sizeof(int16_t***));
    for (int p = 0; p < N; p++){
        int blocks_c = total_mcu*comp[p].h_samp*comp[p].v_samp;
        blocs_reorganises[p] = malloc(blocks_c*sizeof(int16_t**));
        for (int i = 0; i < blocks_c; i++){
            blocs_reorganises[p][i] = malloc(8*sizeof(double*));
        
            for (int j = 0; j < 8; j++){
                blocs_reorganises[p][i][j] = calloc(8, sizeof(double));
            }
        }
    
    
        for (int k = 0; k < total_mcu; k++){  
            int compteur = 0;            
            for (int j = 0; j < 8; j++){
                if (j%2 == 0){
                    for (int w = 0; w <= j; w++ ){
                        blocs_reorganises[p][k][j-w][w] = L[p][k][compteur];
                        compteur++; 
                    }
                }
                else{
                    for (int w = 0; w <= j; w++){
                        blocs_reorganises[p][k][w][j-w] = L[p][k][compteur];
                        compteur++;
                    }
                }
            }
            /* On remplit la seconde moitié de M*/
            for (int j = 1; j < 8; j++){
                if (j%2 == 1){
                    for (int w = 0; w <= 7-j; w++ ){
                        blocs_reorganises[p][k][7-w][j+w] = L[p][k][compteur];
                        compteur++; 
                    }
                }
                else{
                    for (int w = 0; w <= 7-j; w++){
                        blocs_reorganises[p][k][j+w][7-w] = L[p][k][compteur];
                        compteur++;
                    }
                }
            }
        }
    }

    return blocs_reorganises;
}





void zigzag_to_mat(const int16_t in[64], int16_t out[8][8])
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



uint8_t ****build_final_blocks(int16_t          ***deq,    // [c][blk][64] 
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

            int16_t copy1[8][8];
            uint8_t copy2 [8][8];

            zigzag_to_mat(deq[c][b], copy1);
            scalar_idct   (copy1,    copy2);

            for (uint8_t r = 0; r < 8; ++r) {
                final[c][b][r] = malloc(8);
                memcpy(final[c][b][r], copy2[r], 8);
            }
        }
    }
    return final;
}