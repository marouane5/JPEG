#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "ZigZagI.c"

#define PI 3.14159265358979323846


double C(int i){
    double result = 1.0 / sqrt(2.0);
    if (i == 0){
        return result;
    }
    else{
        return 1;
    }

}




double**** idct(int16_t**** blocs_pre_idct, int nbr_blocs, uint16_t N){

    double**** mcu_bloc_idct = malloc(N*sizeof(double***));
    
    for (int i = 0; i < N; i++){
        mcu_bloc_idct[i] = malloc(nbr_blocs*sizeof(double**));
    }

    for (int p = 0; p < N ; p++){
        for (int k = 0; k < nbr_blocs; k++){
            mcu_bloc_idct[p][k] = malloc(8*sizeof(double*));
            for (int w = 0; w < 8; w++){
                mcu_bloc_idct[p][k][w] = malloc(8*sizeof(double));
            }
            for (int i = 0; i < 8; i++){
                for (int j = 0; j< 8; j++){
                    double result = 0.0;
                    for (int lambda = 0; lambda < 8; lambda++){
                        for (int mu = 0; mu < 8; mu++){
                            double coeff_1 = ((2.0*i+1.0)*lambda*PI)/(16.0);
                            double coeff_2 = ((2.0*j+1.0)*mu*PI)/(16.0);    
                            result += C(lambda)*C(mu)*cos(coeff_1)*cos(coeff_2)*blocs_pre_idct[p][k][lambda][mu];               
                        }
                    }
                    mcu_bloc_idct[p][k][i][j] = (1.0/sqrt(16.0))*result;
                }
            }
        }
    }

    return mcu_bloc_idct;

}


uint8_t**** post_idct(double**** mcu_bloc_idct, int nbr_blocs, uint16_t N){

    uint8_t**** mcu_post_idct = malloc(N*sizeof(uint8_t***));
    

    for (int p = 0; p < N; p++) {
        mcu_post_idct[p] = malloc(nbr_blocs * sizeof(uint8_t**));
        for (int k = 0; k < nbr_blocs; k++) {
            mcu_post_idct[p][k] = malloc(8 * sizeof(uint8_t*));
            for (int i = 0; i < 8; i++) {
                mcu_post_idct[p][k][i] = malloc(8 * sizeof(uint8_t));
            }
        }
    }

    for (int p = 0; p < N; p++){
        for (int k = 0; k < nbr_blocs; k++){
            for (int i = 0; i<8; i++){
                for (int j = 0; j < 8; j++){
                    double result = mcu_bloc_idct[p][k][i][j] + 128;
                    if (result > 255){
                        result = 255;
                    }
                    if (result < 0){
                        result = 0;
                    }

                    mcu_post_idct[p][k][i][j] = (uint8_t) round(result);

                }
            }
        }
    }

    return mcu_post_idct;
}



// int main(){

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
//     double** M_idct = idct(M);
//     uint8_t** M_post_idct = post_idct(M_idct);

//     for (int i = 0; i<8; i++){
//         for (int j = 0; j<8; j++){
//             printf("%u ",M_post_idct[i][j]);
//         }
//         printf("\n");
//     }
//     return 0;




// }



// int main(int argc, char **argv){

//     table_de_huffman** dc_ac = arbre_huffman_V2(argv[1]);
//     table_de_huffman* tables_dc = dc_ac[0];
//     table_de_huffman* tables_ac = dc_ac[1];
//     uint16_t* taille = size_picture(argv[1])[0];
//     uint16_t N = *size_picture(argv[1])[2]; 
//     uint16_t* qt_id = size_picture(argv[1])[3];
//     uint16_t nbr_blocs = taille[0]*taille[1]/64;
    
//     ComponentInfo comp[3] = {0};
//     for (uint8_t i = 0; i < N; i++) {
//         comp[i].id = i + 1;            // id (1=y, 2=cb, 3=cr)
//         comp[i].h_samp = 1;            // (on suppose 1 pour simplifier)  apres on peux just les remplires par composantes[2*i+1/2*i]
//         comp[i].v_samp = 1;          
//         comp[i].qt_idx = qt_id[i];    
//     }

//     comp[0].dc_idx = 0;  // Y utilise la table DC 0
//     comp[0].ac_idx = 0;  // Y utilise la table AC 0
//     comp[1].dc_idx = 1;  // Cb utilise la table DC 1
//     comp[1].ac_idx = 1;  // Cb utilise la table AC 1
//     comp[2].dc_idx = 1;  // Cr utilise la table DC 1
//     comp[2].ac_idx = 1;  // Cr utilise la table AC 1

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

//     int16_t*** blocs_Y_Cb_Cr = decode_mcu_blocks_444(tables_dc,
//     tables_ac, argv[1], taille, comp, N);

//     // affichage des résultats pour vérification
//     printf("Composantes de l'image décodées :\n");
    
//     // Calculer le nombre de blocs par composante
//     int width_in_blocks = (taille[0]) / 8;
//     int height_in_blocks = (taille[1]) / 8;
//     int total_blocks = width_in_blocks * height_in_blocks;
    
//     int16_t** quant_tables = extract_quant_table(argv[1]);
    
//     int16_t*** blocs_Y_Cb_Cr_postQ = quantifINV(blocs_Y_Cb_Cr,quant_tables,
//     qt_id, nbr_blocs,N);
    
//     /* TEST table de quantification */

//     // for (int i=0; i<4; i++){
//     //     printf("\ntable %d \n",i);
//     //     if (quant_tables[i] == NULL) break;
//     //     for (int j=0; j<64; j++){
//     //         printf("%d ",quant_tables[i][j]);

//     //     }
//     // }

//     /* TEST quantification inverse */
//     // for (int p=0 ; p < N; p++){
//     //     printf("pour la composante %d\n", p);
//     //     for (int k = 0; k < nbr_blocs; k++){
//     //         printf("pour le bloc %d\n", k);
//     //         for (int i = 0 ; i < 64; i++){
//     //             printf("%02X ", blocs_Y_Cb_Cr_postQ[p][k][i]);
//     //         }
//     //         printf("\n");
//     //     }
//     // }
    
    // /* TEST IDCT */
    // int16_t**** blocs_reorganises = ZigZagI(blocs_Y_Cb_Cr_postQ, nbr_blocs, N);
    // double**** blocs_idct = idct(blocs_reorganises, nbr_blocs, N);
    // for (int p=0; p<N; p++){
    //     printf("composante %d: \n",p);
    //     for (int k=0; k<nbr_blocs; k++){
    //         printf("bloc %d: \n",k);
    //         for (int i=0; i<8; i++){
    //             for (int j = 0; j<8; j++){
    //                 printf("%lf ",blocs_idct[p][k][i][j]);
    //             }
                
    //         }
    //         printf("\n***********************\n");
    //     }
    //     printf("--------------------------\n");
    // }

//     printf("ouf\n");
//     uint8_t**** blocs_post_idct = post_idct(blocs_idct, nbr_blocs, N);



//     for (int p=0; p<N; p++){
//         printf("composante %d: \n",p);
//         for (int k=0; k<nbr_blocs; k++){
//             printf("bloc %d: \n",k);
//             for (int i=0; i<8; i++){
//                 for (int j = 0; j<8; j++){
//                     printf("%02X ",blocs_post_idct[p][k][i][j]);
//                 }
                
//             }
//             printf("\n***********************\n");
//         }
//         printf("--------------------------\n");
//     }


   
// }