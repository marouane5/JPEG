#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

uint8_t** size_picture(char* path){
    FILE*   fichier = fopen(path, "rb");
    uint8_t** resultats = malloc(2*sizeof(uint8_t*));
    if (fichier == NULL){
        printf("failed! \n");
        return 0;
    }
    uint8_t* composantes = malloc(2* sizeof(uint8_t));
    uint8_t* taille = calloc(2, sizeof(uint8_t));
    int curr_b;
    uint8_t a;

    while ((curr_b = fgetc(fichier))!= EOF){
        if (fgetc(fichier)== 0xFF){
            if (fgetc(fichier) == 0xC0){
                /* on ignore les trois octets inutiles*/
                for (int _ = 0; _ <3; _++){
                    fgetc(fichier);
                }

                /*On calcule la hauteur*/ 
                uint8_t hauteur = fgetc(fichier);
                hauteur *= pow(16,2);
                hauteur += (uint8_t) fgetc(fichier);                
                taille[0] = hauteur;
                
                /*On calcule la largeur*/  
                uint8_t largeur = fgetc(fichier);
                largeur *= pow(16,2);
                largeur += (uint8_t) fgetc(fichier);                
                taille[1] = largeur;
                


                
                /* On calcule désormais le facteur d'échantillonage*/
            
                uint8_t N = ((uint8_t) fgetc(fichier));  /* Pour voir si on se trouve dans le cas N = 1 ou N = 3*/

                composantes = calloc(2*N, sizeof(uint8_t));
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



int16_t* extract_quant_table(char* path){
    FILE* fichier = fopen(path, "rb");
    int16_t* quant_table = malloc(64*sizeof(int16_t));
    int curr_b;
    
    if (fichier == NULL){
        printf("failed! \n");
        return 0;
    }
    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            curr_b = fgetc(fichier);  
            if (curr_b == 0xDB){
                for (int _ = 0; _ < 3; _++){
                    fgetc(fichier);
                }
                for (int i = 0; i < 64; i++ ){
                    quant_table[i] = fgetc(fichier);
                }
            }
        }
    }
    return quant_table;
}









uint8_t*** construction_huffman_table_V2(char* path){
    FILE* fichier = fopen(path, "rb");
    uint8_t*** table_huff = malloc(2*sizeof(uint8_t**));
    
    uint8_t** table_couples_huffman_DC = malloc(16*sizeof(uint8_t*));
    uint8_t** table_couples_huffman_AC = malloc(16*sizeof(uint8_t*));
    uint8_t L[16]; // L contient le nombre de codes pour chaque longeur i+1 dans L[i]    

    table_huff[0] = table_couples_huffman_DC;
    table_huff[1] = table_couples_huffman_AC;
    
    for (int j = 0 ; j<16; j++){
        table_couples_huffman_DC[j]= NULL;
        table_couples_huffman_AC[j]= NULL;

    }

    int curr_b;

    if (fichier == NULL){
        printf("failed! \n");
        return 0;
    }

    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            curr_b = fgetc(fichier);  
            if (curr_b == 0xC4){
                for (int _ = 0; _ < 3; _++){
                    fgetc(fichier);
                }
                for (int i = 0; i < 16; i++ ){
                    uint8_t a = fgetc(fichier);
                    L[i] = a;
                }

                for (int i = 0; i<16; i++){
                    table_couples_huffman_DC[i] = malloc((L[i] + 1)*sizeof(uint8_t));
                    table_couples_huffman_DC[i][0] = L[i];
                    /*printf("le nombre d'élements de longeur (DC) %d est %d\n", i+1, table_couples_huffman_DC[i][0]);*/
                    for (int j = 0 ; j<L[i]; j++){
                        uint8_t c = fgetc(fichier);
                        table_couples_huffman_DC[i][j+1] = c;
                        printf("le nombre %02X est codé (DC) en %d bits\n",c,i+1 );
                    }
                }
            /*printf("quand on sort, curr_b = %02X\n", curr_b);*/
            for (int _ = 0; _ < 5; _++){
                fgetc(fichier);
            }
            for (int i = 0; i < 16; i++ ){
                uint8_t a = fgetc(fichier);
                L[i] = a;
            }

            for (int i = 0; i<16; i++){
                table_couples_huffman_AC[i] = malloc((L[i] + 1)*sizeof(uint8_t));
                table_couples_huffman_AC[i][0] = L[i]; /* On positionne le nombre d'élements dans le premier élement de la liste*/
                /*printf("le nombre d'élements de longeur (AC) %d est %d\n", i+1, table_couples_huffman_AC[i][0]);*/
                for (int j = 0 ; j<L[i]; j++){
                    uint8_t c = fgetc(fichier);
                    table_couples_huffman_AC[i][j+1] = c;
                    printf("le nombre %02X est codé (AC) en %d bits\n",c,i+1 );
                }
            }
            }
        }
    }
    return table_huff;
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

table_de_huffman* arbre_huffman_V2(char* path){
    uint8_t*** DHT_DC_AC = construction_huffman_table_V2(path);

    uint8_t** DHT_DC = DHT_DC_AC[0];
    uint8_t** DHT_AC = DHT_DC_AC[1];

    for (int i= 0; i< 16; i++){
        printf("premier élement de DHT_DC[%d] est %d\n", i, DHT_DC[i][0]);
        printf("premier élement de DHT_AC[%d] est %d\n", i, DHT_AC[i][0]);
    }


    int sum_DC = 0; /* Calcul du nombre total de mots codés qu'on a*/
    int sum_AC = 0;
    for (int i = 0; i<16; i++){
        sum_DC += DHT_DC[i][0];
        sum_AC += DHT_AC[i][0];
    }

    printf("le nombre total d'élement codés dans DC est: %d\n", sum_DC);
    printf("le nombre total d'élement codés dans AC est: %d\n", sum_AC);

    huffman_code* L_DC = malloc(sum_DC*sizeof(huffman_code));
    huffman_code* L_AC = malloc(sum_AC*sizeof(huffman_code));


    /* AJOUTE */
    table_de_huffman* Tables_DC_AC = malloc(2*sizeof(table_de_huffman));
    Tables_DC_AC[0].len = sum_DC;
    Tables_DC_AC[0].huff_tab = L_DC;

    Tables_DC_AC[1].len = sum_AC;
    Tables_DC_AC[1].huff_tab = L_AC;


    uint16_t code = 0;
    uint8_t longueur = 1;
    int compteur = 0; /* Pour savoir ou insérer dans L_DC */
    for (int j = 0; j< 16; j++){
        int taille = DHT_DC[j][0];
        printf("la taille pour DHT_DC[%d] est %d\n", j, taille);
        if (taille > 0){
            for (int i = 0; i< taille ; i++){
                char bin[17];
                huffman_code S1;
                S1.symbole = DHT_DC[j][i+1];
                

                dec_to_bin(code, bin);
                strcpy(S1.code, bin);
                S1.longueur = longueur;
                
                printf("le code pour le symbole (DC) %02X est %s et sa longeur est %u\n", S1.symbole, S1.code, S1.longueur);
                code += 1;              
                L_DC[compteur]= S1;
                compteur += 1;
            }
        }
        longueur += 1;
        code *= 2;
    }
    /* On passe à AC */
    longueur = 1;
    code = 0;
    compteur = 0; /* Pour savoir ou insérer dans DHT_AC */
    for (int j =0; j< 16; j++){
        int taille = DHT_AC[j][0];
        printf("la taille pour DHT_AC[%d] est %d\n", j, taille);
        if (taille > 0){
            for (int i = 0; i< taille; i++){
                char bin[17];
                huffman_code S1;
                S1.symbole = DHT_AC[j][i+1];
                

                dec_to_bin(code, bin);
                strcpy(S1.code, bin);
                S1.longueur = longueur;

                printf("le code pour le symbole (DC) %02X est %s et sa longeur est %u\n", S1.symbole, S1.code, S1.longueur);
                code += 1;
                L_AC[compteur]= S1;
                compteur += 1;
            }
        }
        longueur += 1;
        code *= 2;
    }

    return Tables_DC_AC;

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