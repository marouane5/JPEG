#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "decodage.c"



int16_t*** quantifINV(int16_t*** blocs_Y_Cb_Cr, ComponentInfo* comp, uint16_t* taille, int16_t** quant_tables,
    uint16_t* qt_id, uint16_t N){

    uint8_t H_Y = comp[0].h_samp;
    uint8_t V_Y = comp[0].v_samp;


    int mcu_w = (taille[1] + 8*H_Y - 1) / (8*H_Y);
    int mcu_h = (taille[0] + 8*V_Y - 1) / (8*V_Y);
    int total_mcu = mcu_w * mcu_h;

    int16_t*** blocs_Y_Cb_Cr_postQ = malloc(N*sizeof(int16_t**));
    
    for (int p = 0; p < N; p++){
        printf("quantisation\n");
        int nb_blocs = total_mcu * comp[p].h_samp * comp[p].v_samp;
        blocs_Y_Cb_Cr_postQ[p] = malloc(nb_blocs*sizeof(int16_t*));
        for (int k = 0; k < nb_blocs; k++){
            /* On note Q le vecteur contenant les valeurs de quantification */
            blocs_Y_Cb_Cr_postQ[p][k] = malloc(64*sizeof(int16_t));

            for (int i = 0; i< 64 ; i++){
                    blocs_Y_Cb_Cr_postQ[p][k][i] = blocs_Y_Cb_Cr[p][k][i]*quant_tables[qt_id[p]][i];
            }
            
                
        }
    }
    return blocs_Y_Cb_Cr_postQ;

}
