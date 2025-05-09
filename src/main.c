#include <stdio.h>
#include <stdlib.h>

void generate_jpeg(char* filename){
    FILE *file = fopen(filename, "rb");
    if (file == NULL){
        printf("failed! \n");
        return;
    }

    int curr_b;

    while ((curr_b = fgetc(file)) != EOF){
        printf("%02X ", curr_b);
    }
    fclose(file);
    printf("\n");

}



