#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

uint16_t** size_picture(char* path){
    FILE*   fichier = fopen(path, "rb");
    uint16_t** resultats = malloc(2*sizeof(uint16_t*));
    if (fichier == NULL){
        printf("failed! \n");
        return 0;
    }
    uint16_t* composantes = malloc(2* sizeof(uint16_t));
    uint16_t* taille = calloc(2, sizeof(uint16_t));
    int curr_b;
    uint16_t a;

    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            if (fgetc(fichier) == 0xC0){
                /* on ignore les trois octets inutiles*/
                for (int _ = 0; _ <3; _++){
                    fgetc(fichier);
                }

                /*On calcule la hauteur*/ 
                uint16_t hauteur = fgetc(fichier);
                hauteur *= pow(16,2);
                hauteur += (uint16_t) fgetc(fichier);                
                taille[0] = hauteur;
                
                /*On calcule la largeur*/  
                uint16_t largeur = fgetc(fichier);
                largeur *= pow(16,2);
                largeur += (uint16_t) fgetc(fichier);                
                taille[1] = largeur;
                


                
                /* On calcule désormais le facteur d'échantillonage*/
            
                uint16_t N = ((uint16_t) fgetc(fichier));  /* Pour voir si on se trouve dans le cas N = 1 ou N = 3*/

                composantes = calloc(2*N, sizeof(uint16_t));
                for (int i = 0; i< N ; i++){
                    fgetc(fichier);
                    uint8_t octet = fgetc(fichier); // lis 8 bits
                
                    // Extraction des 4 bits de poids fort (les 4 premiers bits)
                    uint8_t demi_octet_fort = (octet >> 4) & 0x0F;
                
                    // Extraction des 4 bits de poids faible (les 4 derniers bits)
                    uint8_t demi_octet_faible = octet & 0x0F;
                    
                    composantes[2*i] = demi_octet_faible;
                    composantes[2*i+1] = demi_octet_fort;
                }

                resultats[0] = taille;
                resultats[1] = composantes;


            }
        }
    }
    fclose(fichier);
    printf("\n");

    return resultats;
}


typedef struct quant_tables{
    uint8_t nbr_quant_tables;
    int16_t** quant_tables_vector;

}quant_tables;   


quant_tables* extract_quant_table(char* path){
    FILE* fichier = fopen(path, "rb");
    int curr_b;
    uint16_t precision;
    uint8_t nbr_tables_q;
    int16_t** quant_tables_vector;
    quant_tables* Q = malloc(sizeof(quant_tables));

    uint8_t taille_section;
    if (fichier == NULL){
        printf("failed! \n");
        return 0 ;
    }
    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            curr_b = fgetc(fichier);  
            if (curr_b == 0xDB){
                for (int _ = 0; _ < 3; _++){
                    taille_section = fgetc(fichier);
                    taille_section *= pow(16,2);
                    taille_section += (uint16_t) fgetc(fichier);
                    precision = (fgetc(fichier)>>4) && 0x0F;
                }
                nbr_tables_q = taille_section/64;
                quant_tables_vector = malloc(nbr_tables_q*sizeof(uint16_t*));
                Q->nbr_quant_tables = nbr_tables_q + 1;

                for (int p = 1; p < nbr_tables_q +1; p++){
                    quant_tables_vector[p]= malloc(64*sizeof(int16_t));
                    if (precision == 0){
                        for (int i = 0; i < 64; i++ ){
                            quant_tables_vector[p][i] = fgetc(fichier);
                        }
                    }
                    else{
                        for (int i = 0; i < 64; i++){
                            uint8_t octet = fgetc(fichier); // lis 8 bits
                        
                            uint8_t demi_octet_fort = (octet >> 4) & 0x0F;
                        
                            uint8_t demi_octet_faible = octet & 0x0F;
                            
                            quant_tables_vector[p][i] = demi_octet_fort*pow(16,2) + demi_octet_faible;
                        }
                    }
                    precision = (fgetc(fichier)>>4) && 0x0F;
                }
                Q->quant_tables_vector = quant_tables_vector;
            }
        }
    }
    return Q;
}









uint8_t**** construction_huffman_table_V2(char* path){
    FILE* fichier = fopen(path, "rb");

    uint8_t**** result = malloc(2*sizeof(uint8_t***)); // tableau contenant 2 cases pour les 4 tableaux de DC et AC
    result[0] = malloc(4*sizeof(uint8_t**)); // chaque case contient 4 tableaux (au maximum)
    result[1] = malloc(4*sizeof(uint8_t**));

    for (int i = 0; i < 4; i++){
        result[0][i] = malloc(16*sizeof(uint8_t*));
        result[1][i] = malloc(16*sizeof(uint8_t*));
            
        for (int j = 0 ; j<16; j++){
            result[0][i][j]= NULL;
            result[1][i][j]= NULL;

        }    

    }
    


    uint8_t L[16]; // L contient le nombre de codes pour chaque longeur i+1 dans L[i]    


    int curr_b;
    int type_composante; // 0 pour DC et 1 pour AC
    int indice;

    if (fichier == NULL){
        printf("failed! \n");
        return 0;
    }

    int nbr_tables_DC = 0;
    int nbr_tables_AC = 0;

    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            curr_b = fgetc(fichier);
            if (curr_b == 0xDA){
                break; // à revoir
            }  
            if (curr_b == 0xC4){
                for (int _ = 0; _ < 2; _++){
                    fgetc(fichier);
                }
                curr_b = fgetc(fichier);
                type_composante = (curr_b >> 4) & 0x0F;
                indice = (curr_b & 0x0F);
                if (type_composante == 0){
                    nbr_tables_DC += 1;
                }

                if (type_composante == 1){
                    nbr_tables_AC += 1;
                }
                printf("******************************************\n");
                printf("Table huffman pour %d et d'indice %d\n", type_composante, indice);
                for (int i = 0; i < 16; i++ ){
                    uint8_t a = fgetc(fichier);
                    L[i] = a;
                }

                for (int i = 0; i<16; i++){
                    result[type_composante][indice][i] = malloc((L[i] + 1)*sizeof(uint8_t));
                    result[type_composante][indice][i][0] = L[i];
                    /*printf("le nombre d'élements de longeur (DC) %d est %d\n", i+1, table_couples_huffman_DC[i][0]);*/
                    for (int j = 0 ; j<L[i]; j++){
                        uint8_t c = fgetc(fichier);
                        result[type_composante][indice][i][j+1] = c;
                        printf("le nombre %02X est codé (DC indice) en %d bits\n",c,i+1 );
                    }
                }

            }
        }
    }
    printf("le nombre de tables DC est %d et AC est %d\n", nbr_tables_DC, nbr_tables_AC);
    for (int i = nbr_tables_DC; i < 4; i++){
        result[0][i] = NULL;
    }
    for (int i = nbr_tables_AC; i < 4; i++){
        result[1][i] = NULL;
    }
    return result;
}


void dec_to_bin(uint16_t n, char* buffer) {
    for (int i = 15; i >= 0; i--) {
        buffer[15 - i] = (n & (1 << i)) ? '1' : '0';
    }
    buffer[16] = '\0';  // null-terminate
}



typedef struct huffman_code{
    uint8_t symbole;
    char code[17];
    uint8_t longueur;

}huffman_code;

typedef struct table_de_huffman{
    int len;
    huffman_code* huff_tab;
}table_de_huffman;

// table_de_huffman* arbre_huffman_V2(char* path){
//     uint8_t**** DHT_DC_AC = construction_huffman_table_V2(path);


//     table_de_huffman** result = malloc(2*sizeof(table_de_huffman*));
//     result[0] = malloc(4*sizeof(table_de_huffman));
//     result[1] = malloc(4*sizeof(table_de_huffman));



//     uint8_t** DHT_DC = DHT_DC_AC[0];
//     uint8_t** DHT_AC = DHT_DC_AC[1];

//     for (int i= 0; i< 16; i++){
//         printf("premier élement de DHT_DC[%d] est %d\n", i, DHT_DC[i][0]);
//         printf("premier élement de DHT_AC[%d] est %d\n", i, DHT_AC[i][0]);
//     }


//     int sum_DC = 0; /* Calcul du nombre total de mots codés qu'on a*/
//     int sum_AC = 0;
//     for (int i = 0; i<16; i++){
//         sum_DC += DHT_DC[i][0];
//         sum_AC += DHT_AC[i][0];
//     }

//     printf("le nombre total d'élement codés dans DC est: %d\n", sum_DC);
//     printf("le nombre total d'élement codés dans AC est: %d\n", sum_AC);

//     huffman_code* L_DC = malloc(sum_DC*sizeof(huffman_code));
//     huffman_code* L_AC = malloc(sum_AC*sizeof(huffman_code));


//     /* AJOUTE */
//     table_de_huffman* Tables_DC_AC = malloc(2*sizeof(table_de_huffman));
//     Tables_DC_AC[0].len = sum_DC;
//     Tables_DC_AC[0].huff_tab = L_DC;

//     Tables_DC_AC[1].len = sum_AC;
//     Tables_DC_AC[1].huff_tab = L_AC;


//     uint16_t code = 0;
//     uint8_t longueur = 1;
//     int compteur = 0; /* Pour savoir ou insérer dans L_DC */
//     for (int j = 0; j< 16; j++){
//         int taille = DHT_DC[j][0];
//         printf("la taille pour DHT_DC[%d] est %d\n", j, taille);
//         if (taille > 0){
//             for (int i = 0; i< taille ; i++){
//                 char bin[17];
//                 huffman_code S1;
//                 S1.symbole = DHT_DC[j][i+1];
                

//                 dec_to_bin(code, bin);
//                 strcpy(S1.code, bin);
//                 S1.longueur = longueur;
                
//                 printf("le code pour le symbole (DC) %02X est %s et sa longeur est %u\n", S1.symbole, S1.code, S1.longueur);
//                 code += 1;              
//                 L_DC[compteur]= S1;
//                 compteur += 1;
//             }
//         }
//         longueur += 1;
//         code *= 2;
//     }
//     /* On passe à AC */
//     longueur = 1;
//     code = 0;
//     compteur = 0; /* Pour savoir ou insérer dans DHT_AC */
//     for (int j =0; j< 16; j++){
//         int taille = DHT_AC[j][0];
//         printf("la taille pour DHT_AC[%d] est %d\n", j, taille);
//         if (taille > 0){
//             for (int i = 0; i< taille; i++){
//                 char bin[17];
//                 huffman_code S1;
//                 S1.symbole = DHT_AC[j][i+1];
                

//                 dec_to_bin(code, bin);
//                 strcpy(S1.code, bin);
//                 S1.longueur = longueur;

//                 printf("le code pour le symbole (DC) %02X est %s et sa longeur est %u\n", S1.symbole, S1.code, S1.longueur);
//                 code += 1;
//                 L_AC[compteur]= S1;
//                 compteur += 1;
//             }
//         }
//         longueur += 1;
//         code *= 2;
//     }

//     return Tables_DC_AC;

// }


table_de_huffman** arbre_huffman_V2(char* path){
    uint8_t**** DHT_DC_AC = construction_huffman_table_V2(path);

    table_de_huffman** result = malloc(2*sizeof(table_de_huffman*));
    result[0] = malloc(4*sizeof(table_de_huffman));
    result[1] = malloc(4*sizeof(table_de_huffman));
    int nbr_elems = 0;

    for (int k = 0; k < 2; k++){
        for (int p = 0; p < 4; p++){
                if (DHT_DC_AC[k][p] == NULL) continue;
                printf("******************************************\n");
                printf("Table huffman pour %d et d'indice %d\n", k, p);
                int sum_DC = 0; /* Calcul du nombre total de mots codés qu'on a*/
                int sum_AC = 0;
                for (int i = 0; i<16; i++){
                    nbr_elems += DHT_DC_AC[k][p][i][0];
                }

                huffman_code* L = malloc(nbr_elems*sizeof(huffman_code));   

                table_de_huffman table_huff;
                table_huff.len = nbr_elems;
                table_huff.huff_tab = L;

                uint16_t code = 0;
                uint8_t longueur = 1;
                int compteur = 0; /* Pour savoir ou insérer dans L_DC */    


                for (int j = 0; j< 16; j++){
                    int taille = DHT_DC_AC[k][p][j][0];
                    printf("la taille pour DHT_DC[%d] est %d\n", j, taille);
                    if (taille > 0){
                        for (int i = 0; i< taille ; i++){
                            char bin[17];
                            huffman_code S1;
                            S1.symbole = DHT_DC_AC[k][p][j][i+1];
                            

                            dec_to_bin(code, bin);
                            strcpy(S1.code, bin);
                            S1.longueur = longueur;
                            
                            printf("le code pour le symbole (DC) %02X est %s et sa longeur est %u\n", S1.symbole, S1.code, S1.longueur);
                            code += 1;              
                            L[compteur]= S1;
                            compteur += 1;
                        }
                    }
                    longueur += 1;
                    code *= 2;
                }
        }
    }
    return result;



}



/* TEST COULEUR */

int main(int argc, char **argv){
    table_de_huffman** result = arbre_huffman_V2(argv[1]);
    return 0;
}



/* TEST */

// int main(int argc, char **argv){
//     uint8_t** resultats = size_picture(argv[1]);
//     uint8_t* taille = resultats[0];
//     printf("la longeur est: %d\n", taille[0]);
//     printf("la hauteur est: %d\n", taille[1]);

//     uint8_t* composantes = resultats[1];
//     printf("la première composante du facteur d'échantillonage est: %d\n", composantes[0]);
//     printf("la deuxieme composante du facteur d'échantillonage est: %d\n", composantes[1]);

//     uint8_t* quant_table = extract_quant_table(argv[1]);
//     /*
//     TEST Pour la table de segmentation
//     for (int i = 0; i<64; i++){
//         printf("%d\n", quant_table[i]);
//     }*/
//     table_de_huffman* L = arbre_huffman_V2(argv[1]);
//     printf("le nombre d'élements DC encodées est: %d, le nombre d'élement AC encodées est %d \n", L[0].len, L[1].len);
//     for (int i = 0; i<L[1].len; i++){
//         printf("symbole %02X; code %s\n", L[1].huff_tab[i].symbole, L[1].huff_tab[i].code);
//     }

//     return 0;
// }