#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "entete_JPEG.c"

int magn_indice_to_coeff(int m, unsigned int idx) {
    if (m == 0)
        return 0;

    int total = 1 << m;      // 2^m
    int half = total / 2;    // Moitie pour les negatifs

    int min_neg = -( (1 << m) - 1 );  // Plus petite valeur negative: ex: m=3 → -7
    int max_neg = -(1 << (m - 1));    // Plus grande valeur negative: ex: m=3 → -4

    if (idx < half) {
        // Partie negative : de -2^m+1 à -2^{m-1}
        return min_neg + idx;
    } else {
        // Partie positive : de 2^{m-1} à 2^m-1
        idx -= half;
        return (1 << (m - 1)) + idx;
    }
}

void extract_mcu_block(const char* filename){
    FILE* file = fopen(filename,"r");
    FILE* new_file = fopen("mcu_hex.txt","w");
    
    int curr_b;
    while ((curr_b = fgetc(file)) != EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xDA){
                uint8_t taille_fort = fgetc(file);
                uint8_t taille_faible = fgetc(file);
                uint16_t taille = taille_faible + pow(16,2)*taille_fort;
                /* taille -2 pour compenser les 2 octets de taille*/
                for (int _=0; _<taille-2; _++){
                    fgetc(file);
                }
                /* tant qu'on est pas encore arrive 
                a la fin de l'image */
                while((curr_b=fgetc(file))!=0xFF){
                    fprintf(new_file,"%02X ",curr_b);
                }
                fprintf(new_file,"\n");
                break;
            }
        }
    }
    fclose(file);
    fclose(new_file);

}

void hex_to_bin(const char *filename)
{   
    extract_mcu_block(filename);
    FILE *in  = fopen("mcu_hex.txt", "r");
    FILE *out = fopen("mcu_bin.txt", "w");
    if (!in || !out) {
        perror("hex_to_bin fopen");
        if (in)  fclose(/* Decoder la composante DC */in);
        if (out) fclose(out);
        return;
    }

    unsigned int byte;                 /* %2x lit dans un unsigned int */
    while (fscanf(in, " %2x", &byte) == 1) {  /* ignore blancs, lit deux hex */
        for (int i = 7; i >= 0; --i)
            fputc( (byte >> i & 1) + '0', out );
    }
    fputc('\n', out);

    fclose(in);
    fclose(out);
}

int16_t* decode_mcu_block(table_de_huffman table_dc, 
    table_de_huffman table_ac, char* filename) {
    
    hex_to_bin(filename);
    int16_t* decoded_mcu_block = malloc(64 * sizeof(int16_t)); 
    FILE* file = fopen("mcu_bin.txt", "r");

    // Récupération des infos
    int len_dc = table_dc.len;
    huffman_code* huff_dc = table_dc.huff_tab;
    int len_ac = table_ac.len;
    huffman_code* huff_ac = table_ac.huff_tab;

    // --- Décodage DC ---
    char seq_cour[17] = {0};
    uint8_t idx = 0;
    uint8_t magnitude;
    uint16_t indice;
    bool code_found = false;
    int curr_b;

    while ((curr_b = fgetc(file)) != EOF) {
        int b = curr_b - '0';
        seq_cour[idx] = curr_b;
        idx++;

        for (int i = 0; i < len_dc; i++) {
            uint8_t len = huff_dc[i].longueur;
            /* si la taille de la sequence courante n'est pas egale
            a celle du code de huff, on passe a l'iteration suivante,
            car on est sur que ca correpond pas a un code*/
            if (idx != len) continue;

            /*on tronque le code de huffman courant*/
            char code_tmp[17];
            for (int j = 0; j < len; j++) {
                code_tmp[j] = huff_dc[i].code[15 - (len - 1 - j)];
            }
            code_tmp[len] = '\0';

            if (strncmp(seq_cour, code_tmp, len) == 0) {
                magnitude = huff_dc[i].symbole;
                indice = 0;
                for (int j = 0; j < magnitude; j++) {
                    b = fgetc(file) - '0';
                    indice = (indice << 1) | b;
                }
                decoded_mcu_block[0] = magn_indice_to_coeff(magnitude, indice);
                code_found = true;
                break;
            }
        }
        if (code_found) break;
    }

    // --- Décodage AC ---
    int compt = 1;
    idx = 0;
    memset(seq_cour, 0, sizeof(seq_cour));
    code_found = false;

    while ((curr_b = fgetc(file)) != EOF && compt < 64) {
        int b = curr_b - '0';
        seq_cour[idx] = curr_b;
        idx++;

        for (int i = 0; i < len_ac; i++) {
            uint8_t len = huff_ac[i].longueur;
            if (idx != len) continue;

            char code_tmp[17];
            for (int j = 0; j < len; j++) {
                code_tmp[j] = huff_ac[i].code[15 - (len - 1 - j)];
            }
            code_tmp[len] = '\0';

            if (strncmp(seq_cour, code_tmp, len) == 0) {
                // Vérifier si c'est EOB
                if (huff_ac[i].symbole == 0x00) {
                    while (compt < 64){ 
                        decoded_mcu_block[compt] = 0;
                        compt++;
                    }
                    fclose(file);
                    return decoded_mcu_block;
                }

                uint8_t coeff_zero = (huff_ac[i].symbole >> 4) & 0x0F;
                magnitude = huff_ac[i].symbole & 0x0F;
                indice = 0;

                for (int j = 0; j < magnitude; j++) {
                    b = fgetc(file) - '0';
                    indice = (indice << 1) | b;
                }

                for (int j = 0; j < coeff_zero && compt < 64; j++) {
                    decoded_mcu_block[compt++] = 0;
                }

                decoded_mcu_block[compt] = magn_indice_to_coeff(magnitude, indice);
                compt++;
                /* on reinitialise seq_cour a 0 pour 
                construire le code suivant*/
                idx = 0;
                memset(seq_cour, 0, sizeof(seq_cour));
                break;
            }
        }
    }
    fclose(file);
    return decoded_mcu_block;
}