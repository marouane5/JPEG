#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

typedef struct huffman_code{
    uint8_t symbole;
    char code[17];
    uint8_t longueur;

}huffman_code;

typedef struct table_de_huffman{
    int len;
    huffman_code* huff_tab;
}table_de_huffman;

typedef struct {
    uint8_t id;        /* component identifier (1=Y,2=Cb,3=Cr) */
    uint8_t h_samp;    /* horizontal sampling factor H  (1–4)     */
    uint8_t v_samp;    /* vertical   sampling factor V  (1–4)     */
    uint8_t qt_idx;    /* quant-table index (0–3)                 */
    uint8_t dc_idx;    /* Huffman DC table index (0–3)            */
    uint8_t ac_idx;    /* Huffman AC table index (0–3)            */
} ComponentInfo;



uint16_t** extract_image_info(char* path){
    /* prend en argument l'image jpeg et renvoie un tableau
    de 4 cases: 
    * case 1: pointe vers un tableau de 2 cases: hauteur, largeur
    * case 2: pointe vers un tableau de 2*N cases: les facteurs d'ech
    (N: nbr de composantes: N=1 ou N=3)
    * case 3: double pointeur vers N 
    * case 4: pointe vers un tableau de N cases: contient l'indice 
    de la table de quantif qui correspond a chaque composante
    */
    FILE*   fichier = fopen(path, "rb");
    uint16_t** resultats = malloc(4*sizeof(uint16_t*));
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
                resultats[0] = taille;
                resultats[1] = composantes;
                resultats[2] = malloc(sizeof(uint16_t));
                *(resultats[2]) = N;
                resultats[3]= id_qt;
            }
        }
    }
    fclose(fichier);
    printf("\n");

    return resultats;
}

void init_component_info(const char *jpeg_path, ComponentInfo comp[3])
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
        comp[i].dc_idx  = 0;                         /* les valeurs réelles  */
        comp[i].ac_idx  = 0;                         /* seront fixées après  */
    }
}


int16_t** extract_quant_tables(char* path){
    /* renvoie un tableau de 4 cases et chaque case pointe
    vers la table de quantification associee ou NULL si celle
    ci n'existe pas */
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

uint8_t**** extract_huffman_info(char* path){
    /* 
    * renvoie un tableau de 2 cases pour DC et AC
    * chaque case pointe vers un tableau de 4 cases 
    (4 tableaux par composante au maximum)
    * chaque case de ce dernier tableau pointe vers un 
    tableau de 16 cases (chaque case i correpond a une longueur de i+1)
    * chaque case i de ce dernier tableau pointe vers un tableau 
    de n+1 elements (n nombre de symboles dont le code est de 
    longueur i+1): le premier element de ce tableau fournit
    la valeur de n et les cases restantes indiquent les symboles 
    dont le code est de longueur i+1
    */
    FILE* fichier = fopen(path, "rb");

    // tableau contenant 2 cases pour les 4 tableaux de DC et AC
    uint8_t**** result = malloc(2*sizeof(uint8_t***)); 
    // chaque case contient 4 tableaux (on 4 tableaux au maximum)
    result[1] = malloc(4*sizeof(uint8_t**));
    result[0] = malloc(4*sizeof(uint8_t**)); 

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
                    result[type_composante][indice][i] = malloc((L[i] + 1)*sizeof(uint8_t));
                    result[type_composante][indice][i][0] = L[i];
                    for (int j = 0 ; j<L[i]; j++){
                        uint8_t c = fgetc(fichier);
                        result[type_composante][indice][i][j+1] = c;
                    }
                }

            }
        }
    }
    for (int i = nbr_tables_DC; i < 4; i++){
        result[0][i] = NULL;
    }
    for (int i = nbr_tables_AC; i < 4; i++){
        result[1][i] = NULL;
    }
    return result;
}

void dec_to_bin(uint16_t n, char* buffer){
    for (int i=15; i>=0; i--){
        buffer[15-i] = (n & (1 << i)) ? '1' : '0';
    }
    buffer[16] = '\0';
}

table_de_huffman** construction_arbre_huffman(char* path){
    /* renvoie un tableau de deux cases pour DC et AC
    * chaque case pointe vers un tableu de 4 cases
    * chaque case de ce dernier tableau est une 
    structure table_de_huffman (qui se construit en utilisant
    les informations fournies par la fonction precedante)
    */
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

uint8_t** extract_huff_idx(char* filename){
    FILE* file = fopen(filename, "rb");
    int curr_b;

    while((curr_b = fgetc(file))!= EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xDA){
                /* on ignore les deux octets qui indiquent
                la longueur de la section*/
                fgetc(file); fgetc(file); 
                uint8_t N = fgetc(file);
                uint8_t** huff_idx_tables = malloc(N*sizeof(uint8_t*));
                uint8_t idx_comp;
                uint8_t octet; uint8_t idx_huff_dc; uint8_t idx_huff_ac;
                
                while (int i=0; i<N; i++){
                    idx_comp = fgetc(file);
                    octet = fgetc(file);
                    idx_huff_dc = (octet >> 4) & 0x0F;
                    idx_huff_ac = octet & 0x0F;
                }
            }
        }

    }

    fclose(file);
}
