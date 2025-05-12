#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "ZigZagI.c"

#define PI 3.14159265358979323846


double C(int i){
    double result = 1.0 / sqrt(2.0);
    if (i == 0){
        return result;
    }
    else{
        return 1;
    }

}




double** idct(int16_t** mcu_bloc){

    double** mcu_bloc_idct = malloc(8*sizeof(double*));
    
    for (int i = 0; i < 8; i++){
        mcu_bloc_idct[i] = malloc(8*sizeof(double));
    }

    double result = 0;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j< 8; j++){
            for (int lambda = 0; lambda < 8; lambda++){
                for (int mu = 0; mu < 8; mu++){
                    double coeff_1 = ((2.0*i+1.0)*lambda*PI)/(16.0);
                    double coeff_2 = ((2.0*j+1.0)*mu*PI)/(16.0);    
                    result += C(lambda)*C(mu)*cos(coeff_1)*cos(coeff_2)*mcu_bloc[lambda][mu];               
                }
            }
            mcu_bloc_idct[i][j] = (1.0/sqrt(16.0))*result;
            result = 0;
        }
    }

    return mcu_bloc_idct;

}


uint8_t** post_idct(double** mcu_bloc_idct){

    uint8_t** mcu_post_idct = malloc(8*sizeof(int8_t*));
    
    for (int i = 0; i < 8; i++){
        mcu_post_idct[i] = malloc(8*sizeof(int8_t));
    }

    for (int i = 0; i<8; i++){
        for (int j = 0; j < 8; j++){
            double result = mcu_bloc_idct[i][j] + 128;
            if (result > 255){
                result = 255;
            }
            if (result < 0){
                result = 0;
            }

            mcu_post_idct[i][j] = (uint8_t) round(result);

        }
    }
    return mcu_post_idct;
}



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

//     int16_t** M = ZigZagI(matrice);
//     double** M_idct = idct(M);
//     uint8_t** M_post_idct = post_idct(M_idct);

//     for (int i = 0; i<8; i++){
//         for (int j = 0; j<8; j++){
//             printf("%u ",M_post_idct[i][j]);
//         }
//         printf("\n");
//     }
//     return 0;




// }