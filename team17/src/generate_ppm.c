#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "generate_ppm.h"
#include "entete_JPEG.h"

void generate_image(uint16_t* taille, uint16_t* dim_reel , uint8_t**** mcu_blocks, uint16_t N, char* jpeg_path){
    /* génére le fichier pgm ou ppm à partir des mcu caculées*/

    // Récupérer uniquement le nom de fichier (sans le chemin)
    const char* filename = strrchr(jpeg_path, '/');
    if (filename) {
        filename++; // sauter le slash
    } else {
        filename = jpeg_path;
    }

    // Copier le nom de fichier sans le chemin
    char output_filename[256];
    strncpy(output_filename, filename, 255);

    // Supprimer l'extension .jpeg ou .jpg
    size_t len = strlen(output_filename);
    if (len >= 5 && strcmp(output_filename + len - 5, ".jpeg") == 0) {
        output_filename[len - 5] = '\0';
    } else if (len >= 4 && strcmp(output_filename + len - 4, ".jpg") == 0) {
        output_filename[len - 4] = '\0';
    }

    // Ajouter la bonne extension
    if (N == 1) {
        strcat(output_filename, ".pgm");
    } else {
        strcat(output_filename, ".ppm");
    }

    FILE* file = fopen(output_filename, "w");
    fprintf(file, (N == 1) ? "P5\n" : "P6\n");

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

