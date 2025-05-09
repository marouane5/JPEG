#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

uint8_t** taille_image(char* filename){
    FILE *file = fopen(filename , "r");
    uint8_t **res = malloc(2*sizeof(uint8_t*));
    uint8_t* dim;
    uint8_t* facteurs_ech;

    int curr_b;
    while ((curr_b = fgetc(file)) != EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xC0){
                /* les dimensions */
                dim = malloc(2*sizeof(uint8_t));
                /* on ignore les 3 premiers octets */
                for (int _ = 0; _<3; _++){
                    fgetc(file);
                }
                /* on sauvegarde la hauteur et la largeur*/
                dim[0] = fgetc(file)*pow(16,2);
                dim[0] += fgetc(file);
                dim[1] = fgetc(file)*pow(16,2);
                dim[1] += fgetc(file);

                res[0] = dim;
                
                /* les facteurs d'echantillonnage */
                /* n = Nombre de composantes (Ex : 3 pour 
                le YCbCr, 1 pour les niveaux de gris) */
                int n = fgetc(file);
                facteurs_ech = malloc(2*n*sizeof(uint8_t));
                for (int i=0; i<n; i++){
                    fgetc(file);
                    int fact = fgetc(file);
                    uint8_t fact_fort = (fact >> 4) & 0x0F;
                    facteurs_ech[2*i] = fact_fort;
                    uint8_t fact_faible = fact & 0x0F;
                    facteurs_ech[2*i+1] = fact_faible;
                    fgetc(file);
                }
                res[1] = facteurs_ech;
            }
        }
    }
    return res;
}


uint8_t* quantif_table(char* filename){
    FILE* file = fopen(filename,"r");
    uint8_t* tab = malloc(64*sizeof(uint8_t));

    int curr_b;
    while ((curr_b = fgetc(file))!= EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xDB){
                /* on ignore les 3 premiers octets*/
                for (int _=0; _<3; _++){
                    fgetc(file);
                }
                for (int i=0; i<64; i++){
                    tab[i] = fgetc(file);
                }
            }
        }
    }
    return tab;
}



int main(){
    uint8_t** res = taille_image("../images/invader.jpeg");
    uint8_t* dim = res[0];
    uint8_t* facteurs_ech = res[1];

    printf("%d ",dim[0]);
    printf("%d ",dim[1]);
    printf("\n");

    printf("%d ",facteurs_ech[0]);
    printf("%d ",facteurs_ech[1]);
    printf("\n");

    uint8_t* tab = quantif_table("../images/invader.jpeg");
    for (int i=0; i<64; i++){
        printf("%d ",tab[i]);
    }
    printf("\n");
    return 0;
}