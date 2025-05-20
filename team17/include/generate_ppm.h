#ifndef GENERATE_PPM_H    // Garde d'inclusion : évite les inclusions multiples
#define GENERATE_PPM_H

#include <stdint.h>

void generate_image(uint16_t* taille, uint16_t* dim_reel , uint8_t**** mcu_blocks, uint16_t N, char* jpeg_path);
/* génére le fichier pgm ou ppm à partir des MCUs calculées*/


#endif
