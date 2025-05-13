#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "quantifInv.c"
#include "IDCT.c"
#include "decodage.c"



void dec_to_bin_octet(uint8_t n, char* buffer) {
    for (int i = 7; i >= 0; i--) {
        buffer[7 - i] = (n & (1 << i)) ? '1' : '0'; 
    }
    buffer[8] = '\0';  // null-terminate
}   


void generate_pgm(uint8_t* dim , uint8_t** mcu_block){

    FILE* file = fopen("image_etape1.pgm","w");
    fprintf(file,"P5\n");
    uint8_t h = dim[0];
    uint8_t w = dim[1];
    fprintf(file,"%u %u\n",w,h);
    fprintf(file,"255\n");
    for (int i=0; i<8; i++){
        for (int j=0; j<8; j++){    
            fprintf(file,"%u ",mcu_block[i][j]);
        }
    fprintf(file,"\n");
    }
    fclose(file);
   
   }

void generate_pgm_V2(uint16_t* dim , uint8_t*** mcu_blocks){

    FILE* file = fopen("image_etape3.pgm","w");
    fprintf(file,"P5\n");
    uint16_t h = dim[0];
    uint16_t w = dim[1];
    fprintf(file,"%u %u\n",w,h);
    fprintf(file,"255\n");


    for (int p = 0; p < h/8.0; p++){
        for (int i = 0; i < 8; i++){
            for (int k = 0; k < w/8.0; k++ ){
                for (int j = 0; j < 8; j++){
                    fwrite(&mcu_blocks[(p*w/8)+k][i][j], sizeof(uint8_t), 1, file);
                    // fprintf(file,"\\x%02X",mcu_blocks[(p*w/8)+k][i][j]);
                }
            }
        }
    }
    fprintf(file,"\n");
    fclose(file);
    }   




int main(int argc, char **argv){
    uint16_t** resultats = size_picture(argv[1]);
    uint16_t* taille = resultats[0];
    int nombre_blocks = (int) (taille[0]*taille[1])/64.0;

    table_de_huffman* table = arbre_huffman_V2(argv[1]);
    int16_t* quant_table = extract_quant_table(argv[1]);

    table_de_huffman table_dc = table[0];
    table_de_huffman table_ac = table[1];
    int16_t** coeff = decode_mcu_block(table_dc,table_ac,argv[1],taille);

    int16_t** coeff_post_quant = malloc(nombre_blocks*sizeof(int16_t*));
    for (int i = 0; i < nombre_blocks; i++){
        coeff_post_quant[i] = malloc(64*sizeof(int16_t));
        int16_t* coeff_i_post = quantifINV(coeff[i],quant_table);

        for (int j = 0; j < 64; j++){
            coeff_post_quant[i][j] = coeff_i_post[j];
        }
    }

    int16_t*** M_zigzag = malloc(nombre_blocks*sizeof(int16_t**));
    for (int i = 0; i < nombre_blocks; i++){
        M_zigzag[i] = ZigZagI(coeff_post_quant[i]);
    }

    


    double*** M_idct = malloc(nombre_blocks*sizeof(int16_t**));
    for (int i = 0; i < nombre_blocks; i++){
            M_idct[i] = idct(M_zigzag[i]);
    }



    uint8_t*** M_post_idct = malloc(nombre_blocks*sizeof(int16_t**));
    for (int i = 0; i < nombre_blocks; i++){
            M_post_idct[i] = post_idct(M_idct[i]);
    }

    for (int k = 0; k < nombre_blocks; k++){
        for (int i = 0; i < 8; i++){
            for (int j = 0; j < 8 ;j++){
                printf("%02X ", M_post_idct[k][i][j]);
            }
        }
        printf("\n");
        printf("\n");

    }


    printf("la hauteur est %d\n", taille[0]);
    printf("la largeur est %d\n", taille[1]);
    generate_pgm_V2(taille, M_post_idct);
    return 0;   
}   