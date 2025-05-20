#ifndef CONVERSION_RGB_H    // Garde d'inclusion : évite les inclusions multiples
#define CONVERSION_RGB_H

#include <stdint.h>

double**** conversion_rgb(uint8_t**** blocs_Y_Cb_Cr, int nb_blocs);
    /* convertit les valeurs finales de Y, Cb et Cr selon les formules données
    vers des triplets RGB */
    
uint8_t**** saturation_rgb(double**** blocs_RGB, int nbr_blocs);
    /* sature les valeurs trouvées et renvoie des entiers prêts à utiliser */

void free_conversion_rgb(double**** blocsRGB, int nb_blocs);

void free_saturation_rgb(uint8_t**** blocs_rgb_sature, int nbr_blocs);

#endif
