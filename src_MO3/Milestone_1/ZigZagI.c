#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/* Fonction ZigZagI pour le cas d'une liste contenant 64 élements */
int16_t** ZigZagI(int16_t* L){
    /* On initialise la matrice résultante M*/
    
    int16_t** M = malloc(8*sizeof(int16_t*));
    for (int i = 0; i < 8; i++){

        M[i] = malloc(8*sizeof(int16_t));

    } 
    
    int compteur = 0;
    for (int j = 0; j < 8; j++){
        if (j%2 == 0){
            for (int k = 0; k <= j; k++ ){
                M[j-k][k] = L[compteur];
                compteur++; 
            }
        }
        else{
            for (int k = 0; k <= j; k++){
                M[k][j-k] = L[compteur];
                compteur++;
            }
        }
    }
    /* On remplit la seconde moitié de M*/
    for (int j = 1; j < 8; j++){
        if (j%2 == 1){
            for (int k = 0; k <= 7-j; k++ ){
                M[7-k][j+k] = L[compteur];
                compteur++; 
            }
        }
        else{
            for (int k = 0; k <= 7-j; k++){
                M[j+k][7-k] = L[compteur];
                compteur++;
            }
        }
    }
    return M;
}


/* TEST ZigZagI*/
// int main(void){

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

//     int16_t** M = ZigZagI(matrice);
//     for (int i = 0; i<8; i++){
//         for (int j = 0; j<8; j++){
//             printf("l'élement de la %deme ligne et %deme colonne est %d\n", i+1, j+1, M[i][j]);
//         }
//     }
//     return 0;



// }
