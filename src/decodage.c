#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
// #include "decodage.h"

typedef struct huffman_code{
    uint8_t symbole;
    uint16_t code;

}huffman_code;

typedef struct table_de_huffman
{
    int len;
    huffman_code* huff_tab;
}table_de_huffman;


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

void extract_mcu_block(char* filename){
    FILE* file = fopen(filename,"r");
    FILE* new_file = fopen("");
    
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
                    printf("%02X ",curr_b);
                }
                printf("\n");
                break;
            }
        }
    }

}

void hex_to_bin(char* filename){
    /* prend en argument un fichier qui contient 
    un bloc MCU codee en hex et le transforme 
    en binaire pour pouvoir le decoder*/
    FILE* file = fopen(filename,"r");
    int curr_b;
    while ((curr_b = fgetc(file))!=EOF){
        /* on affiche l'octet en binaire*/
        for (int i = 7; i >= 0; i--) {
            printf("%d", (curr_b >> i) & 1);
        }
    }
    printf("\n");

}

int16_t* decode_mcu_block(table_de_huffman table_ac, 
    table_de_huffman table_dc, char* filename) {
    int len_dc = table_dc.len;
    huffman_code* huff_dc = table_dc.huff_tab;

    int len_ac = table_ac.len;
    huffman_code* huff_ac = table_ac.huff_tab;

    FILE* file = fopen(filename, "r");
    int16_t* decoded_mcu_block = malloc(64 * sizeof(int16_t)); 
    uint16_t indice = 0;
    uint8_t magnitude;
    bool code_found = false;
    int curr_b;
    uint16_t seq_cour = 0; // la sequence construite pour le moment

    /* Decoder la composante DC */
    while ((curr_b = fgetc(file)) != EOF) {
        
        int b = curr_b - '0';
        seq_cour = (seq_cour << 1) | b;
        printf("seq_cour %d\n",seq_cour);

        for (int i = 0; i < len_dc; i++) {
            if (huff_dc[i].code == seq_cour) {
                printf("code %d\n",huff_dc[i].code);
                magnitude = huff_dc[i].symbole;
                for (int j = 0; j < magnitude; j++) {
                    b = fgetc(file);
                    indice = (indice << 1) | (b - '0');
                }
                printf("magnitude %d\n",magnitude);
                printf("indice %d\n",indice);
                int coeff = magn_indice_to_coeff(magnitude, indice);
                decoded_mcu_block[0] = coeff;
                printf("coeff %d\n",coeff);
                code_found = true;
                break;
            }
        }
        if (code_found) break;
    }

    /* Decoder la composante AC */
    printf("AC\n");
    int compt = 1;
    seq_cour = 0;
    code_found = false;

    while ((curr_b = fgetc(file)) != EOF && compt < 64) {
        int b = curr_b - '0';
        seq_cour = (seq_cour << 1) | b;
        printf("seq_cour %d\n",seq_cour);
        if (seq_cour == 0) { // EOB
            while (compt < 64) {
                decoded_mcu_block[compt] = 0;
                compt++;
            }
            code_found = true;
            break;
        }

        for (int i = 0; i < len_ac; i++) {
            if (huff_ac[i].code == seq_cour){
                printf("code %d\n",huff_dc[i].code);
                uint8_t coeff_zero = (huff_ac[i].symbole >> 4) & 0x0F;
                magnitude = (huff_ac[i].symbole) & 0x0F;
                indice = 0;

            for (int j = 0; j < magnitude; j++) {
                b = fgetc(file);
                indice = (indice << 1) | (b - '0');
            }

            for (int j = 0; j < coeff_zero && compt < 64; j++) {
                decoded_mcu_block[compt] = 0;
                compt++;
            }
            int16_t coeff = magn_indice_to_coeff(magnitude, indice);
            decoded_mcu_block[compt] = coeff; 
            printf("magnitude %d\n",magnitude);
            printf("indice %d\n",indice);
            printf("coeff %d\n",coeff);
            compt++;
            code_found = true;
            break;
            }
        }
        if (code_found){
            seq_cour = 0;
            code_found = false;
        }
    }

    fclose(file); 
    return decoded_mcu_block;
}


int main(int argc, char **argv) {
        // Table DC pour luminance (classe 0)
    huffman_code huff_dc[] = {
        {0x00, 0b000},   // magnitude 0
        {0x01, 0b0010},  // magnitude 1
        {0x02, 0b0011},  // magnitude 2
        {0x03, 0b0100},  // magnitude 3
        {0x04, 0b0101},  // magnitude 4
        {0x05, 0b0110},  // magnitude 5
        {0x06, 0b1110},  // magnitude 6
    };

    table_de_huffman table_dc = {
        .len = 7,
        .huff_tab = huff_dc
    };

    // Table AC pour luminance (simplifiee)
    huffman_code huff_ac[] = {
        {0x00, 0b00},       // EOB
        {0x011, 0b100},     // (0,1)
        {0x021, 0b101},     // (0,2)
        {0x031, 0b110},     // (0,3)
        {0x012, 0b1110},    // (1,2)
        {0x011, 0b11110},   // (1,1)
        {0x001, 0b111110},  // (0,0), RLE 1 zero
    };

    table_de_huffman table_ac = {
        .len = 7,
        .huff_tab = huff_ac
    };

    int16_t* coeff = decode_mcu_block(table_ac, table_dc, "bin.txt");

    // --- Affichage des resultats ---
    printf("Coefficients decode:\n");
    for (int i = 0; i < 64; i++) {
        printf("%d ", coeff[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }

    free(coeff);
    return 0;
}