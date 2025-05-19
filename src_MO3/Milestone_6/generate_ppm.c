#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "conversion_rgb.c"
#include "ZigZagI.c"


void generate_pgm_V3_troncature(uint16_t* taille, uint16_t* dim_reel , uint8_t**** mcu_blocks, uint16_t N){
    FILE* file;
    if (N==1){
        file = fopen("imagegrise.pgm","w");
        fprintf(file,"P5\n");
    }
    else{
        file = fopen("imagecouleur.ppm","w");
        fprintf(file,"P6\n");
    
    }
    uint16_t h = dim_reel[0];
    uint16_t w = dim_reel[1];

    uint16_t h_complet = taille[0];
    uint16_t w_complet = taille[1];


    fprintf(file,"%u %u\n",w,h);
    fprintf(file,"255\n");

    
    for (int p = 0; p < h_complet/8 ; p++){ //Changement fait: h/8 au lieu de h/8.0
        // printf("slm\n");
        // printf("%d\n", p);
        if (p == h_complet/8 -1){
            for (int i = 0; i < 8 - (h_complet - h); i++){
                for (int k = 0; k < w_complet/8 ; k++ ){   //Changement fait: w/8 au lieu de w/8.0
                    if (k == w_complet/8 -1){
                        for (int j = 0; j < 8 - (w_complet - w); j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                        }
                    }
                    else {
                        for (int j = 0; j < 8; j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }// printf("%u ", mcu_blocks[(p*w/8)+k][i][j]);
                            // fprintf(file,"\\x%02X",mcu_blocks[(p*w/8)+k][i][j]);
                        
                    }
                    }
                }
            }
        }
        else {
            for (int i = 0; i < 8; i++){
                for (int k = 0; k < w_complet/8 ; k++ ){   //Changement fait: w/8 au lieu de w/8.0
                    if (k == w_complet/8 -1){
                        for (int j = 0; j < 8 - (w_complet - w); j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                        }
                    }
                    else {
                        for (int j = 0; j < 8; j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                            // printf("%u ", mcu_blocks[(p*w/8)+k][i][j]);
                            // fprintf(file,"\\x%02X",mcu_blocks[(p*w/8)+k][i][j]);
                        
                    }
                    }
                }
            }
        }
    }
    printf("yoooo\n");
    fprintf(file,"\n");
    fclose(file);
    }   









int main(int argc, char **argv){
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
        return 1;
    }


    uint16_t** resultats = size_picture(argv[1]);
    uint16_t* taille = resultats[0];
    uint16_t N = *(resultats[2]); 
    uint16_t* composantes = resultats[1];
    uint16_t* qt_id = resultats[3];

    ComponentInfo comp[3] = {0};  // Maximum 3 composantes (Y, Cb, Cr)
    
    // on remplit les structures ComponentInfo
    for (uint8_t i = 0; i < N; i++) {
        comp[i].id = i + 1;            // id (1=y, 2=cb, 3=cr)
        comp[i].h_samp = composantes[2*i+1];            // (on suppose 1 pour simplifier)  apres on peux just les remplires par composantes[2*i+1/2*i]
        comp[i].v_samp = composantes[2*i];          
        comp[i].qt_idx = qt_id[i];    
    }
    
    // extraction des tables de Huffman
    table_de_huffman** tables = arbre_huffman_V2(argv[1]);
    

    table_de_huffman tables_dc[2] = {tables[0][0], tables[0][1]};  // Tables DC
    table_de_huffman tables_ac[2] = {tables[1][0], tables[1][1]};  // Tables AC
    
  
    comp[0].dc_idx = 0;  // Y utilise la table DC 0
    comp[0].ac_idx = 0;  // Y utilise la table AC 0
    comp[1].dc_idx = 1;  // Cb utilise la table DC 1
    comp[1].ac_idx = 1;  // Cb utilise la table AC 1
    comp[2].dc_idx = 1;  // Cr utilise la table DC 1
    comp[2].ac_idx = 1;  // Cr utilise la table AC 1


   


    // uint16_t h_complete = taille[0];
    // uint16_t l_complete = taille[1];

    uint16_t* taille_complete = malloc(2*sizeof(uint16_t));
    

   /* main.c ---------------------------------------------------------*/
    uint16_t l_complete = ((taille[1] + 8 * comp[0].h_samp - 1)
                /(8 * comp[0].h_samp)) * (8 * comp[0].h_samp);

    uint16_t h_complete = ((taille[0] + 8 * comp[0].v_samp - 1) 
                    /(8 * comp[0].v_samp)) * (8 * comp[0].v_samp);




    taille_complete[0] = h_complete;
    taille_complete[1] = l_complete;
    int nbr_blocs = h_complete*l_complete/64;

    
    
    // decodage des MCU pour obtenir les composantes Y, Cb, Cr
    int16_t*** bloc_Y_Cb_Cr = decode_mcu_blocks_444(tables_dc, tables_ac, argv[1], taille_complete, comp, N);
    
    if (bloc_Y_Cb_Cr == NULL) {
        fprintf(stderr, "Erreur lors du décodage des MCU\n");
        return 1;
    }
    
    // affichage des résultats pour vérification
    printf("Composantes de l'image décodées :\n");
    
    // Calculer le nombre de blocs par composante
    // int width_in_blocks = l_complete / 8;
    // int height_in_blocks = h_complete / 8;
    // int total_blocks = width_in_blocks * height_in_blocks;


    int mcu_w = (taille[1] + 8 * comp[0].h_samp - 1) / (8 * comp[0].h_samp);
    int mcu_h = (taille[0] + 8 * comp[0].v_samp - 1) / (8 * comp[0].v_samp);
    int total_blocks = mcu_w * mcu_h*comp[0].h_samp*comp[0].v_samp;
    
    

    printf("total_blocks = %d , nbr_blocks = %u\n", total_blocks, nbr_blocs);


    int16_t** quant_tables = extract_quant_table(argv[1]);
    
    int16_t*** blocs_Y_Cb_Cr_postQ = quantifINV(bloc_Y_Cb_Cr, comp,taille, quant_tables,
    qt_id, N);


    printf("la quanti se fait");





    

    printf("le zigzag se fait");

    //double**** blocs_idct = idct(blocs_reorganises, comp, total_blocks,N);
    uint8_t**** blocs_final = build_final_blocks(blocs_Y_Cb_Cr_postQ, comp, mcu_w, mcu_h,N);
    
    printf("l'etap avant upsampling marche \n");

    uint8_t**** blocs_finalv = upsampling(blocs_final,comp, mcu_w, mcu_h,N);

    printf(" aprse upsampling \n");

    // Pour chaque composante
    for (uint8_t c = 0; c < N; c++) {
        const char* comp_name;
        switch (c) {
            case 0: comp_name = "Y"; break;
            case 1: comp_name = "Cb"; break;
            case 2: comp_name = "Cr"; break;
            default: comp_name = "je ne sai pas c'est quoi cette couleur de merde"; break;
        }
        
        printf("Composante %s :\n", comp_name);
        
        // afficher les 5 prem blocks
        int blocks_to_show = total_blocks;
        for (int b = 0; b < blocks_to_show; b++) {
            printf("  Bloc %d :\n", b);
            for (int i = 0; i < 8; i++) {
                printf("    ");
                for (int j = 0; j < 8; j++) {
                    int idx = i * 8 + j;
                    printf("%4u ", blocs_finalv[c][b][i][j]);
                }
                printf("\n");
            }
        }
    }

    int total_blocks_Y = mcu_w * mcu_h * comp[0].h_samp * comp[0].v_samp;

    
    
    double**** blocs_RGB;
    uint8_t**** blocs_rgb_sature;
    if (N == 3){
        blocs_RGB = conversion_rgb(blocs_finalv, total_blocks_Y);
        blocs_finalv = saturation_rgb(blocs_RGB, total_blocks_Y);

    }

    printf("la hauteur est %d\n", taille[0]);
    printf("la largeur est %d\n", taille[1]);

    generate_pgm_V3_troncature(taille_complete, taille, blocs_finalv, N);
    free(taille_complete);
    return 0;   
}


