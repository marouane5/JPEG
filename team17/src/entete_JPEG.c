#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "entete_JPEG.h"


uint16_t** extract_image_info(char* path){
    FILE*   fichier = fopen(path, "rb");
    uint16_t** infos = malloc(4*sizeof(uint16_t*));
    if (fichier == NULL){
        printf("failed! \n");
        return NULL;
    }
    uint16_t* composantes = NULL;
    uint16_t* taille = calloc(2, sizeof(uint16_t));
    int curr_b;

    // le marqueur de prelule
    int prec = fgetc(fichier);
    int curr = fgetc(fichier);
    
    if (prec != 0xFF || curr != 0xD8){
        printf("error");
        return NULL;
    }

    // le marqueur de app0

    prec = fgetc(fichier);
    curr= fgetc(fichier);
    if (prec != 0xFF ){
        printf("error");
        return NULL;
    }

    if (curr == 0xE0 ){
        uint16_t taille_section = (fgetc(fichier)<<8 ) + fgetc(fichier);
        for ( int i =0; i < taille_section-2;++i ){
            fgetc(fichier);
        }
    }

    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            if (fgetc(fichier) == 0xC0){
                /* on ignore les deux octets qui indiquent
                la longueur de la section*/
                fgetc(fichier); fgetc(fichier); 

                //Précision en bits par composante, toujours 8 pour le baseline (voir annex A)
                uint8_t precision = fgetc(fichier);  
                if (precision != 8){
                    perror("erreur! en baseline la precision est toujours 8\n");
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

                /* On calcule désormais les facteur d'échantillonage*/
            
                uint16_t N = ((uint16_t) fgetc(fichier));  /* Pour voir si on se trouve dans le cas N = 1 ou N = 3*/

                if (N>4 || N <1){
                    printf("nombre de conposantes de couleurs est invalide");
                }
                composantes = calloc(2*N, sizeof(uint16_t));
                uint16_t* id_qt = calloc(N, sizeof(uint16_t));
                for (int i = 0; i< N ; i++){
                    fgetc(fichier); /* = identifiant_composante */
                    uint8_t octet = fgetc(fichier); // lis 8 bits
                
                    // Extraction des 4 bits de poids fort (les 4 premiers bits)
                    uint8_t demi_octet_fort = (octet >> 4) & 0x0F;
                
                    // Extraction des 4 bits de poids faible (les 4 derniers bits)
                    uint8_t demi_octet_faible = octet & 0x0F;
                    
                    composantes[2*i] = demi_octet_faible;
                    composantes[2*i+1] = demi_octet_fort;
                    id_qt[i] = fgetc(fichier);
                }
                infos[0] = taille;
                infos[1] = composantes;
                infos[2] = malloc(sizeof(uint16_t));
                *(infos[2]) = N;
                infos[3]= id_qt;
            }
        }
    }
    fclose(fichier);
    return infos;
}

void init_component_info(const char *jpeg_path, ComponentInfo comp[3], uint8_t** huff_idx_tables)
{
  
    uint16_t **infos      = extract_image_info((char *)jpeg_path);
    uint16_t  N           = *infos[2];        /* nombre de composantes */
    uint16_t *composantes =  infos[1];        /* tableau [V1,H1,V2,H2…] */
    uint16_t *qt_id       =  infos[3];        /* index des tables Q     */


    for (uint8_t i = 0; i < N && i < 3; ++i) {
        comp[i].id      = i + 1;                     /* 1 = Y, 2 = Cb, 3 = Cr */
        comp[i].h_samp  = composantes[2*i + 1];      /* H_i du SOF           */
        comp[i].v_samp  = composantes[2*i];          /* V_i du SOF           */
        comp[i].qt_idx  = qt_id[i];                  /* table de quantif     */
        comp[i].dc_idx  = huff_idx_tables[i][0]; 
        comp[i].ac_idx  = huff_idx_tables[i][1];           
    }
}


void free_image_info(uint16_t **infos) {
    if (infos == NULL) return;

    for (int i = 0; i < 4; i++) {
        free(infos[i]);
    }
    free(infos);
}


int16_t** extract_quant_tables(char* path){
    FILE* fichier = fopen(path, "rb");
    int16_t** quant_tables = calloc(4,sizeof(int16_t*));
    int curr_b;
    
    if (fichier == NULL){
        printf("failed! \n");
        return NULL;
    }
    while ((curr_b = fgetc(fichier))!= EOF){
        if (curr_b== 0xFF){
            curr_b = fgetc(fichier);  
            if (curr_b == 0xDB){
                int16_t len = (fgetc(fichier) <<8)+ fgetc(fichier);
                len -=2;
                while(len>0){
                    int info = fgetc(fichier);
                    len -=1;
                    int table_indice = info & 0x0F;

                    if(table_indice > 4){
                        printf("le nombre de quant table is more than expected");
                        return NULL;
                    }
                    quant_tables[table_indice] = malloc(64 * sizeof(int16_t));
                    if (info >> 4 ==1){  // si la presition est 1 alors on a 16 bits ( voire l'annexe A)
                        for (int i =0; i < 64; i++){
                            quant_tables[table_indice][i] = (fgetc(fichier)<< 8) + fgetc(fichier);
                        }
                        len -=128;
                    }
                    else {
                        for (int i =0; i < 64; i++){
                            quant_tables[table_indice][i] =  fgetc(fichier);
                        }
                        len -=64;

                    }
                }
            if (len !=0){
                printf("il y a une erreur qlq par dans le fichier jpg/jpeg"); //le numbre de valeurs de table de quantification est incoherant
                return NULL;
            }
            }
        
        }
    }
    fclose(fichier);
    return quant_tables;
}

void free_quant_tables(uint16_t **quant_tables) {
    if (quant_tables == NULL) return;

    for (int idx = 0; idx < 2; idx++) {
        free(quant_tables[idx]);
    }
    free(quant_tables);
}


uint8_t**** extract_huffman_info(char* path){
    FILE* fichier = fopen(path, "rb");

    // tableau contenant 2 cases pour les 4 tableaux de DC et AC
    uint8_t**** huff_info = malloc(2*sizeof(uint8_t***)); 
    // chaque case contient 4 tableaux (on 4 tableaux au maximum)
    huff_info[1] = malloc(4*sizeof(uint8_t**));
    huff_info[0] = malloc(4*sizeof(uint8_t**)); 

    for (int i = 0; i < 4; i++){
        huff_info[0][i] = malloc(16*sizeof(uint8_t*));
        huff_info[1][i] = malloc(16*sizeof(uint8_t*));
            
        for (int j = 0 ; j<16; j++){
            huff_info[0][i][j]= NULL;
            huff_info[1][i][j]= NULL;
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
            if (curr_b == 0xDA) break; 
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
                for (int i = 0; i < 16; i++ ){
                    uint8_t a = fgetc(fichier);
                    L[i] = a;
                }

                for (int i = 0; i<16; i++){
                    huff_info[type_composante][indice][i] = malloc((L[i] + 1)*sizeof(uint8_t));
                    huff_info[type_composante][indice][i][0] = L[i];
                    for (int j = 0 ; j<L[i]; j++){
                        uint8_t c = fgetc(fichier);
                        huff_info[type_composante][indice][i][j+1] = c;
                    }
                }

            }
        }
    }
    for (int i = nbr_tables_DC; i < 4; i++){
        huff_info[0][i] = NULL;
    }
    for (int i = nbr_tables_AC; i < 4; i++){
        huff_info[1][i] = NULL;
    }
    return huff_info;
}

void free_huff_info(uint8_t**** huff_info){
    for (int comp=0; comp<2; comp++){
        for (int idx_tab = 0; idx_tab<4; idx_tab++){
            for (int len=0; len<16; len++){
                free(huff_info[comp][idx_tab][len]);
            }
            free(huff_info[comp][idx_tab]);
        }
        free(huff_info[comp]);
    }
    free(huff_info);

}



void dec_to_bin(uint16_t n, char* buffer){
    for (int i=15; i>=0; i--){
        buffer[15-i] = (n & (1 << i)) ? '1' : '0';
    }
    buffer[16] = '\0';
}

table_de_huffman** construction_arbre_huffman(char* path){
    uint8_t**** DHT_DC_AC = extract_huffman_info(path);

    table_de_huffman** result = malloc(2*sizeof(table_de_huffman*));
    result[0] = malloc(4*sizeof(table_de_huffman));
    result[1] = malloc(4*sizeof(table_de_huffman));
    int nbr_elems = 0;

    for (int k = 0; k < 2; k++){
        for (int p = 0; p < 4; p++){
                if (DHT_DC_AC[k][p] == NULL) continue;
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
                    if (taille > 0){
                        for (int i = 0; i< taille ; i++){
                            char bin[17];
                            huffman_code S1;
                            S1.symbole = DHT_DC_AC[k][p][j][i+1];
                            dec_to_bin(code, bin);
                            strcpy(S1.code, bin);
                            S1.longueur = longueur;

                            code += 1;              
                            L[compteur]= S1;
                            compteur += 1;
                        }
                    }
                    longueur += 1;
                    code *= 2;
                }
                result[k][p] = table_huff;

        }
    }
    return result;
}

void free_arbre_huffman(table_de_huffman** arbre_huffman){
    for (int comp=0; comp<2; comp++){
        for (int idx_table; idx_table<4; idx_table++){
        free(arbre_huffman[comp][idx_table].huff_tab);
        }
        free(arbre_huffman[comp]);
    }
    free(arbre_huffman);

}


uint8_t** extract_huff_idx(char* filename){
    FILE* file = fopen(filename, "rb");
    int curr_b;
    uint8_t** huff_idx_tables;

    while((curr_b = fgetc(file))!= EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xDA){
                /* on ignore les deux octets qui indiquent
                la longueur de la section*/
                fgetc(file); fgetc(file); 
                uint8_t N = fgetc(file);
                huff_idx_tables = malloc(N*sizeof(uint8_t*));
                uint8_t idx_comp;
                uint8_t octet; uint8_t idx_huff_dc; uint8_t idx_huff_ac;
                
                for (int _=0; _<N; _++){
                    idx_comp = fgetc(file);
                    huff_idx_tables[idx_comp - 1] = malloc(2*sizeof(uint8_t));
                    octet = fgetc(file);
                    idx_huff_dc = (octet >> 4) & 0x0F;
                    huff_idx_tables[idx_comp - 1][0] = idx_huff_dc;
                    idx_huff_ac = octet & 0x0F;
                    huff_idx_tables[idx_comp - 1][1] = idx_huff_ac;
                    
                }
                break;
            }
        }
    }
    fclose(file);
    return huff_idx_tables;
    
}

void free_huff_idx(uint8_t** huff_idx_tables, uint8_t N){
    for (int i=0; i<N; i++){
        free(huff_idx_tables[i]);
    }
    free(huff_idx_tables);

}