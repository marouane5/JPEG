#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "quantifInv.c"


/* Fonction ZigZagI pour le cas d'une liste contenant 64 élements */
int16_t**** ZigZagI(int16_t*** L, uint16_t nbr_blocs, uint16_t N){
    
    int16_t**** blocs_reorganises = malloc(N*sizeof(uint16_t***));
    for (int p = 0; p < N; p++){
        blocs_reorganises[p] = malloc(nbr_blocs*sizeof(uint16_t**));
    }
    
    for (int p = 0; p < N; p++){
        for (int k = 0; k < nbr_blocs; k++){                
            /* On initialise la matrice résultante M*/
            
            int16_t** M = malloc(8*sizeof(int16_t*));
            
            for (int i = 0; i < 8; i++){

                M[i] = malloc(8*sizeof(int16_t));

            } 
            
            int compteur = 0;
            for (int j = 0; j < 8; j++){
                if (j%2 == 0){
                    for (int k = 0; k <= j; k++ ){
                        M[j-k][k] = L[p][k][compteur];
                        compteur++; 
                    }
                }
                else{
                    for (int k = 0; k <= j; k++){
                        M[k][j-k] = L[p][k][compteur];
                        compteur++;
                    }
                }
            }
            /* On remplit la seconde moitié de M*/
            for (int j = 1; j < 8; j++){
                if (j%2 == 1){
                    for (int k = 0; k <= 7-j; k++ ){
                        M[7-k][j+k] = L[p][k][compteur];
                        compteur++; 
                    }
                }
                else{
                    for (int k = 0; k <= 7-j; k++){
                        M[j+k][7-k] = L[p][k][compteur];
                        compteur++;
                    }
                }
            }
            blocs_reorganises[p][k] = M;
        }
    }

    return blocs_reorganises;
}






/* TEST ZigZagI*/
// int main(void){

//     int16_t matrice[64] = {
//         124, 0, -6, -333, 0, -284, 0, -293,
//         0, -202, 128, 0, -117, 0, 0, 0,
//         106, 0, -96, 0, -90, -138, 0, 284,
//         0, 69, 0, -20, 0, -131, 0, 25,
//         0, 19, 0, 221, 0, 26, 0, 255,
//         0, 154, 0, 0, 4, 0, 125, 0,
//         -88, 0, -167, 0, 20, 0, 0, -481,
//         0, -71, 0, 244, 0, 0, -196, 0
//     };

//     int16_t** M = ZigZagI(matrice);
//     for (int i = 0; i<8; i++){
//         for (int j = 0; j<8; j++){
//             printf("l'élement de la %deme ligne et %deme colonne est %d\n", i+1, j+1, M[i][j]);
//         }
//     }
//     return 0;



// }


int main(int argc, char **argv){

    table_de_huffman** dc_ac = arbre_huffman_V2(argv[1]);
    table_de_huffman* tables_dc = dc_ac[0];
    table_de_huffman* tables_ac = dc_ac[1];
    uint16_t* taille = size_picture(argv[1])[0];
    uint16_t N = *size_picture(argv[1])[2]; 
    uint16_t* qt_id = size_picture(argv[1])[3];
    uint16_t nbr_blocs = taille[0]*taille[1]/64;
    
    ComponentInfo comp[3] = {0};
    for (uint8_t i = 0; i < N; i++) {
        comp[i].id = i + 1;            // id (1=y, 2=cb, 3=cr)
        comp[i].h_samp = 1;            // (on suppose 1 pour simplifier)  apres on peux just les remplires par composantes[2*i+1/2*i]
        comp[i].v_samp = 1;          
        comp[i].qt_idx = qt_id[i];    
    }

    /* AJOUTE MO3*/
    uint16_t h_reel = taille[0];
    uint16_t l_reel = taille[1];

    uint16_t* taille_reel = malloc(2*sizeof(uint16_t));
    taille_reel[0] = h_reel;
    taille_reel[1] = l_reel;

    if (taille[1]%8 != 0 && taille[0]%8 != 0){
        taille[1] = 8*((l_reel/8) +1);
        taille[0] = 8*((h_reel/8) +1);
    }
    
    else if (taille[1]%8 !=0){
        taille[1] = 8*((l_reel/8) +1);
        
    }
    else if (taille[0]%8 !=0){
        taille[0] = 8*((h_reel/8) +1);
    }
    else{
        printf("Pas besoin de troncature!\n");
    }

    int16_t*** blocs_Y_Cb_Cr = decode_mcu_blocks_444(tables_dc,
    tables_ac, argv[1], taille, comp, N);
    
    int16_t** quant_tables = extract_quant_table(argv[1]);
    
    int16_t*** blocs_Y_Cb_Cr_postQ = quantifINV(blocs_Y_Cb_Cr,quant_tables,
    qt_id, nbr_blocs,N);
    
    // int16_t**** blocs_reorganises = ZigZagI(blocs_Y_Cb_Cr_postQ, nbr_blocs,N);
    
    for (int i=0; i<4; i++){
        printf("\ntable %d \n",i);
        if (quant_tables[i] == NULL) break;
        for (int j=0; j<64; j++){
            printf("%d ",quant_tables[i][j]);

        }
    }


    // for (int p=0; p<N; p++){
    //     printf("composante %d: \n",p);
    //     for (int k=0; k<nbr_blocs; k++){
    //         printf("bloc %d: \n",k);
    //         for (int i=0; i<64; i++){
    //             printf("%02X ",blocs_Y_Cb_Cr_postQ[p][k][i]);
    //         }
    //         printf("\n***********************\n");
    //     }
    //     printf("--------------------------\n");
    // }

}