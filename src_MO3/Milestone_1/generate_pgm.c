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
    fprintf(file,"P2\n");
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



int main(int argc, char **argv){
    uint8_t** resultats = size_picture(argv[1]);
    uint8_t* taille = resultats[0];

    table_de_huffman* table = arbre_huffman_V2(argv[1]);
    int16_t* quant_table = extract_quant_table(argv[1]);

    table_de_huffman table_dc = table[0];
    table_de_huffman table_ac = table[1];
    int16_t* coeff = decode_mcu_block(table_dc,table_ac,argv[1]);

    int16_t* L_res = quantifINV(coeff, quant_table);
    int16_t** M = ZigZagI(L_res);

    double** M_idct = idct(M);
    uint8_t** M_post_idct = post_idct(M_idct);



    generate_pgm(taille, M_post_idct);
    return 0;   
}