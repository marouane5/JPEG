#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


double**** conversion_rgb(uint8_t**** blocs_Y_Cb_Cr, int nb_blocs){
    double**** blocs_RGB = malloc(3*sizeof(double***));
    for (int p = 0; p < 3; p++){
        blocs_RGB[p] = malloc(nb_blocs*sizeof(double**));
        
        for (int i = 0; i < nb_blocs; i++){
            blocs_RGB[p][i] = malloc(8*sizeof(double*));
        
            for (int j = 0; j < 8; j++){
                blocs_RGB[p][i][j] = malloc(8*sizeof(double));
            }
        }
    }

    for (int k = 0; k < nb_blocs; k++){
        //printf("on calcule le %deme bloc\n", k);
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){
                blocs_RGB[0][k][i][j] = blocs_Y_Cb_Cr[0][k][i][j] + 1.402*(blocs_Y_Cb_Cr[2][k][i][j] - 128);
                blocs_RGB[1][k][i][j] = blocs_Y_Cb_Cr[0][k][i][j] - 0.34414*(blocs_Y_Cb_Cr[1][k][i][j] - 128) 
                -0.71414*(blocs_Y_Cb_Cr[2][k][i][j] - 128);       
                blocs_RGB[2][k][i][j] = blocs_Y_Cb_Cr[0][k][i][j] + 1.772*(blocs_Y_Cb_Cr[1][k][i][j] - 128);
            }
        }

    }


    return blocs_RGB;    
}

uint8_t**** saturation_rgb(double**** blocs_RGB, int nbr_blocs){
    
    uint8_t**** blocs_rgb_sature = malloc(3*sizeof(uint8_t***));
    

    for (int p = 0; p < 3; p++) {
        blocs_rgb_sature[p] = malloc(nbr_blocs * sizeof(uint8_t**));
        for (int k = 0; k < nbr_blocs; k++) {
            blocs_rgb_sature[p][k] = malloc(8 * sizeof(uint8_t*));
            for (int i = 0; i < 8; i++) {
                blocs_rgb_sature[p][k][i] = malloc(8 * sizeof(uint8_t));
            }
        }
    }
    
    for (int p = 0; p < 3; p++ ){
        for (int k = 0; k < nbr_blocs; k++){
            for (int i = 0; i < 8; i++){
                for (int j = 0; j < 8; j++){
                    double result = blocs_RGB[p][k][i][j];
                    if (result > 255){
                        result = 255;
                    }
                    if (result < 0){
                        result = 0;
                    }
                    blocs_rgb_sature[p][k][i][j] = (uint8_t) round(result);
                }
            }
        }
    }

    return blocs_rgb_sature;
}