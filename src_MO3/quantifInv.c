#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


uint8_t** quantifINV(uint8_t** M, uint8_t** Q){
    /* On note Q la matrice de quantification 8x8 */
    uint8_t** M_res = malloc(8*sizeof(uint8_t*));

    for (int i = 0; i<8; i++){
        M_res[i] = malloc(8*sizeof(uint8_t));
    }

    for (int i = 0; i<8 ; i++){
        for (int j = 0; j<8; j++){
            M_res[i][j] = M[i][j]*Q[i][j];
        }
    }

    return M_res;
}


