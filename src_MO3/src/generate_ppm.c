#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "generate_ppm.h"
#include "entete_JPEG.h"

void generate_image(uint16_t* taille, uint16_t* dim_reel , uint8_t**** mcu_blocks, uint16_t N){
    /* génére le fichier pgm ou ppm à partir des mcu caculées*/

    char output_filename[256];
    size_t len = strlen(jpeg_path);

    // Identifier l'extension
    int ext_len = 0;
    if (len >= 5 && strcmp(jpeg_path + len - 5, ".jpeg") == 0) {
        ext_len = 5;
    } else if (len >= 4 && strcmp(jpeg_path + len - 4, ".jpg") == 0) {
        ext_len = 4;
    } else {
        fprintf(stderr, "Erreur : extension inconnue\n");
        return;
    }

    // Copier le nom de base (sans l'extension)
    strncpy(output_filename, jpeg_path, len - ext_len);
    output_filename[len - ext_len] = '\0';

    char* extension;
    if (N==1) extension = ".pgm";
    if (N==3) extension = ".ppm";

    strcat(output_filename, extension);
    FILE* file = fopen(output_filename, "w");

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


