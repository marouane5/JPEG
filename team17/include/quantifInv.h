#ifndef QUANTIFINV_H    // Garde d'inclusion : évite les inclusions multiples
#define QUANTIFINV_H


#include <stdint.h>
#include "decodage.h"

int16_t*** quantifINV(int16_t*** blocs_Y_Cb_Cr,
                        ComponentInfo* comp,
                        uint16_t* taille,
                        int16_t** quant_tables,
                        uint16_t* qt_id,
                        uint16_t N);
/* prend en argument les tables de quantification et les différents blocs et 
effectue le produit entre les différents élements et renvoie le tableau des 
trois élements*/

void free_quantif_inv(int16_t*** blocs_Y_Cb_Cr_postQ,
                    ComponentInfo* comp,
                    uint16_t* taille,
                    uint16_t N);


#endif
