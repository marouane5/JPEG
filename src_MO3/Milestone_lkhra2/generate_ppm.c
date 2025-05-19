#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "generate_ppm.h"
#include "entete_JPEG.h"

void generate_image(uint16_t* taille, uint16_t* dim_reel , uint8_t**** mcu_blocks, uint16_t N){
    /* génére le fichier pgm ou ppm à partir des mcu caculées*/
    FILE* file;
    if (N==1){
        file = fopen("image_grise.pgm","w");
        fprintf(file,"P5\n");
    }
    else{
        file = fopen("image_couleur.ppm","w");
        fprintf(file,"P6\n");
    
    }
    uint16_t h = dim_reel[0];
    uint16_t w = dim_reel[1];

    uint16_t h_complet = taille[0];
    uint16_t w_complet = taille[1];


    fprintf(file,"%u %u\n",w,h);
    fprintf(file,"255\n");

    
    for (int p = 0; p < h_complet/8 ; p++){ 
        if (p == h_complet/8 -1){
            for (int i = 0; i < 8 - (h_complet - h); i++){
                for (int k = 0; k < w_complet/8 ; k++ ){ 
                    if (k == w_complet/8 -1){
                        for (int j = 0; j < 8 - (w_complet - w); j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                        }
                    }
                    else {
                        for (int j = 0; j < 8; j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                        
                    }
                    }
                }
            }
        }
        else {
            for (int i = 0; i < 8; i++){
                for (int k = 0; k < w_complet/8 ; k++ ){ 
                    if (k == w_complet/8 -1){
                        for (int j = 0; j < 8 - (w_complet - w); j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }
                        }
                    }
                    else {
                        for (int j = 0; j < 8; j++){
                            for (int component = 0; component < N; component++){
                                fwrite(&mcu_blocks[component][(p*w_complet/8)+k][i][j], sizeof(uint8_t), 1, file);
                            }   
                    }
                    }
                }
            }
        }
    }
    fprintf(file,"\n");
    fclose(file);
    }   


