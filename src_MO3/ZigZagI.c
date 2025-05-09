#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


/* Fonction ZigZagI pour le cas d'une liste contenant 64 élements */
uint8_t** ZigZagI(uint8_t* L){
    /* On initialise la matrice résultante M*/
    
    uint8_t** M = malloc(8*sizeof(uint8_t*));
    for (int i = 0; i < 8; i++){

        M[i] = malloc(8*sizeof(uint8_t));

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
int main(void){
    uint8_t* L = malloc(64*sizeof(uint8_t));

    for (int i = 0; i<64; i++ ){
        L[i] = i;
    } 

    uint8_t** M = ZigZagI(L);
    for (int i = 0; i<8; i++){
        for (int j = 0; j<8; j++){
            printf("l'élement de la %ueme ligne et %ueme colonne est %u\n", i+1, j+1, M[i][j]);
        }
    }



}
