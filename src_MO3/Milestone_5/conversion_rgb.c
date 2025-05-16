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
        printf("on calcule le %deme bloc\n", k);
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++){
                blocs_RGB[0][k][i][j] = blocs_Y_Cb_Cr[0][k][i][j] + 1.402*(blocs_Y_Cb_Cr[2][k][i][j] - 128);
                blocs_RGB[1][k][i][j] = blocs_Y_Cb_Cr[0][k][i][j] - 0.34414*(blocs_Y_Cb_Cr[1][k][i][j] - 128) -0.71414*(blocs_Y_Cb_Cr[2][k][i][j] - 128);       
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



// int main(int argc, char **argv) {
//     if (argc < 2) {
//         fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
//         return 1;
//     }
    

//     uint16_t** resultats = size_picture(argv[1]);
//     uint16_t* taille = resultats[0];
//     uint16_t N = *(resultats[2]); 
//     uint16_t* composantes = resultats[1];
//     uint16_t* qt_id = resultats[3];
    
//     /* AJOUTE MO3*/
//     uint16_t h_reel = taille[0];
//     uint16_t l_reel = taille[1];

//     uint16_t* taille_reel = malloc(2*sizeof(uint16_t));
//     taille_reel[0] = h_reel;
//     taille_reel[1] = l_reel;

//     if (taille[1]%8 != 0 && taille[0]%8 != 0){
//         taille[1] = 8*((l_reel/8) +1);
//         taille[0] = 8*((h_reel/8) +1);
//     }
    
//     else if (taille[1]%8 !=0){
//         taille[1] = 8*((l_reel/8) +1);
        
//     }
//     else if (taille[0]%8 !=0){
//         taille[0] = 8*((h_reel/8) +1);
//     }
//     else{
//         printf("Pas besoin de troncature!\n");
//     }

//     ComponentInfo comp[3] = {0};  // Maximum 3 composantes (Y, Cb, Cr)
    
//     // on remplit les structures ComponentInfo
//     for (uint8_t i = 0; i < N; i++) {
//         comp[i].id = i + 1;            // id (1=y, 2=cb, 3=cr)
//         comp[i].h_samp = 1;            // (on suppose 1 pour simplifier)  apres on peux just les remplires par composantes[2*i+1/2*i]
//         comp[i].v_samp = 1;          
//         comp[i].qt_idx = qt_id[i];    
//     }
    
//     // extraction des tables de Huffman
//     table_de_huffman** tables = arbre_huffman_V2(argv[1]);
    

//     table_de_huffman tables_dc[2] = {tables[0][0], tables[0][1]};  // Tables DC
//     table_de_huffman tables_ac[2] = {tables[1][0], tables[1][1]};  // Tables AC
    
  
//     comp[0].dc_idx = 0;  // Y utilise la table DC 0
//     comp[0].ac_idx = 0;  // Y utilise la table AC 0
//     comp[1].ac_idx = 1;  // Cb utilise la table AC 1
//     comp[2].dc_idx = 1;  // Cr utilise la table DC 1
//     comp[2].ac_idx = 1;  // Cr utilise la table AC 1
    
//     // decodage des MCU pour obtenir les composantes Y, Cb, Cr
//     int16_t*** components = decode_mcu_blocks_444(tables_dc, tables_ac, argv[1], taille, comp, N);
    
//     if (components == NULL) {
//         fprintf(stderr, "Erreur lors du décodage des MCU\n");
//         return 1;
//     }
    
//     // affichage des résultats pour vérification
//     printf("Composantes de l'image décodées :\n");
    
//     // Calculer le nombre de blocs par composante
//     int width_in_blocks = (taille[0] + 7) / 8;
//     int height_in_blocks = (taille[1] + 7) / 8;
//     int total_blocks = width_in_blocks * height_in_blocks;
    
//     // Pour chaque composante
//     for (uint8_t c = 0; c < N; c++) {
//         const char* comp_name;
//         switch (c) {
//             case 0: comp_name = "Y"; break;
//             case 1: comp_name = "Cb"; break;
//             case 2: comp_name = "Cr"; break;
//             default: comp_name = "je ne sai pas c'est quoi cette couleur de merde"; break;
//         }
        
//         printf("Composante %s :\n", comp_name);
        
//         // afficher les 5 prem blocks
//         int blocks_to_show = total_blocks;
//         for (int b = 0; b < blocks_to_show; b++) {
//             printf("  Bloc %d :\n", b);
//             for (int i = 0; i < 64; i++) {
//                 printf("%4d ", components[c][b][i]);
//             }
//             printf("\n");
//         }
//     }
    
//     // lib de la mémoire
//     for (uint8_t c = 0; c < N; c++) {
//         for (int j = 0; j < total_blocks; j++) {
//             free(components[c][j]);
//         }
//         free(components[c]);
//     }
//     free(components);
    
    
//     return 0;
// }



