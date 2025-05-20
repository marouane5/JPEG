#ifndef DECODAGE_H    // Garde d'inclusion : évite les inclusions multiples
#define DECODAGE_H

#include <stdint.h>
#include <stdio.h>
#include "entete_JPEG.h"
#define MAX_HUFF_LEN 16      /* per JPEG spec */


int magn_indice_to_coeff(int m, int idx);
    /* renvoie le coefficient associé à la magnitude 
    et l'indice données en argument */

void extract_mcu_blocks(char* filename);
    /* génère un fichier mcu_hex.txt contenant uniquement le 
    contenu de tout les blocs de l'image JPEG */

void hex_to_bin(char *filename);

void decode_one_block(
        FILE *bitstream,          //flux  binaire genere
        int16_t coef[64],         // tableau destination 8×8             
        int16_t *prev_dc,         // valeur DC précédente de la composante
        table_de_huffman dc_tab,  // table Huffman DC
        table_de_huffman ac_tab);  // table Huffman AC

uint8_t ***pixel_dup(uint8_t **in,
                uint8_t       h_factor,
                uint8_t       v_factor);
        /* duplique les blocs dans le cas de sur-échantillonage*/

int16_t*** decode_mcu_blocks(
        table_de_huffman *tables_dc, table_de_huffman *tables_ac,
        char *filename,
        uint16_t *taille,
        ComponentInfo *comp,
        uint8_t nb_comp);
        /* décode les MCUs et renvoie un tableau de 3 cases, chacune correspondant à une
        composante (Y, Cb, Cr) et contenant des vecteurs de 64 valeurs en nombre de celui
        des blocs
        */

void free_mcu_blocks(int16_t*** mcu_blocks,
    uint16_t *taille,
    ComponentInfo *comp,
    uint8_t N);

uint8_t ****upsampling(uint8_t ****in,
                       const ComponentInfo comp[/*N*/],
                       int                 mcu_w,
                       int                 mcu_h,
                       uint8_t             N);
        /* Utilise pixel_dup pour effectuer le sur-echantillonage*/


void free_upsampling(uint8_t ****MCUs,
        ComponentInfo comp[/*N*/],
        int                 mcu_w,
        int                 mcu_h,
        uint8_t             N);



#endif
