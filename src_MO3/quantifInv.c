#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>




int16_t* quantifINV(int16_t* M, int16_t* Q){
    /* On note Q le vecteur contenant les valeurs de quantification */
    int16_t* L_res = malloc(64*sizeof(int16_t));

    for (int i = 0; i< 64 ; i++){
            L_res[i] = M[i]*Q[i];
    }

    return L_res;
}

/* TEST */
// int main(){

//     int16_t matrice[64] = {
//         124, 0, -6, -333, 0, -284, 0, -293,
//         0, -202, 128, 0, -117, 0, 0, 0,
//         106, 0, -96, 0, -90, -138, 0, 284,
//         0, 69, 0, -20, 0, -131, 0, 25,
//         0, 19, 0, 221, 0, 26, 0, 255,
//         0, 154, 0, 0, 4, 0, 125, 0,
//         -88, 0, -167, 0, 20, 0, 0, -481,
//         0, -71, 0, 244, 0, 0, -196, 0
//     };

//     int16_t Q[64];
//     for (int i = 0; i < 64; i++){
//         Q[i] = 2;
//     }
//     int16_t* L_res = quantifINV(matrice, Q);
//     for (int i = 0; i < 64; i++){
//         printf("%d  ", L_res[i]);
//     }
//     printf("\n");


//     return 0;
// }