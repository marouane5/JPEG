#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h> 
#include "generate_ppm.h"
#include "decodage.h"
#include "quantifInv.h"
#include "zigzaginv_idct.h"
#include "conversion_rgb.h"



int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
        return 1;
    }


    uint16_t** resultats = extract_image_info(argv[1]);
    uint16_t* taille = resultats[0];
    uint16_t N = *(resultats[2]); 
    uint16_t* qt_id = resultats[3];

    ComponentInfo comp[3] = {0};  // Maximum 3 composantes (Y, Cb, Cr)
    
    uint8_t** huff_idx_tables = extract_huff_idx(argv[1]);
    init_component_info(argv[1], comp, huff_idx_tables);
    
    // extraction des tables de Huffman
    table_de_huffman** tables = construction_arbre_huffman(argv[1]);
    

    table_de_huffman tables_dc[2] = {tables[0][0], tables[0][1]};  // Tables DC
    table_de_huffman tables_ac[2] = {tables[1][0], tables[1][1]};  // Tables AC
    

    uint16_t* taille_complete = malloc(2*sizeof(uint16_t));
    

   /* main.c ---------------------------------------------------------*/
    uint16_t l_complete = ((taille[1] + 8 * comp[0].h_samp - 1)
                /(8 * comp[0].h_samp)) * (8 * comp[0].h_samp);

    uint16_t h_complete = ((taille[0] + 8 * comp[0].v_samp - 1) 
                    /(8 * comp[0].v_samp)) * (8 * comp[0].v_samp);




    taille_complete[0] = h_complete;
    taille_complete[1] = l_complete;

    
    // decodage des MCU pour obtenir les composantes Y, Cb, Cr
    int16_t*** bloc_Y_Cb_Cr = decode_mcu_blocks(tables_dc, tables_ac, argv[1], taille_complete, comp, N);
    
    if (bloc_Y_Cb_Cr == NULL) {
        fprintf(stderr, "Erreur lors du décodage des MCU\n");
        return 1;
    }
    
    
    int mcu_w = (taille[1] + 8 * comp[0].h_samp - 1) / (8 * comp[0].h_samp);
    int mcu_h = (taille[0] + 8 * comp[0].v_samp - 1) / (8 * comp[0].v_samp);    


    int16_t** quant_tables = extract_quant_tables(argv[1]);
    
    int16_t*** blocs_Y_Cb_Cr_postQ = quantifINV(bloc_Y_Cb_Cr, comp,taille, quant_tables,
    qt_id, N);



    //double**** blocs_idct = idct(blocs_reorganises, comp, total_blocks,N);
    uint8_t**** blocs_final = blocks_post_zigzaginv_idct(blocs_Y_Cb_Cr_postQ, comp, mcu_w, mcu_h,N);
    

    uint8_t**** final_blocks = upsampling(blocs_final,comp, mcu_w, mcu_h,N);


    int total_blocks_Y = mcu_w * mcu_h * comp[0].h_samp * comp[0].v_samp;

    
    
    double**** blocs_RGB;
    if (N == 3){
        blocs_RGB = conversion_rgb(final_blocks, total_blocks_Y);
        final_blocks = saturation_rgb(blocs_RGB, total_blocks_Y);

    }


    generate_image(taille_complete, taille, final_blocks, N);
    free(taille_complete);
    return 0;   
}