#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "entete_JPEG_2.c"
#define MAX_HUFF_LEN 16      /* per JPEG spec */

int get_next_bit(FILE *f)               /* returns '0', '1' or EOF */
{
    int c;
    do {
        c = fgetc(f);
        if (c == EOF) return EOF;
    } while (c != '0' && c != '1');     /* skip \n, spaces, … */
    return c;
}

int magn_indice_to_coeff(int m, unsigned int idx) {
    if (m == 0)
        return 0;

    int total = 1 << m;      // 2^m
    int half = total / 2;    // Moitie pour les negatifs

    int min_neg = -( (1 << m) - 1 );  // Plus petite valeur negative: ex: m=3 → -7
    int max_neg = -(1 << (m - 1));    // Plus grande valeur negative: ex: m=3 → -4

    if (idx < half) {
        // Partie negative : de -2^m+1 à -2^{m-1}
        return min_neg + idx;
    } else {
        // Partie positive : de 2^{m-1} à 2^m-1
        idx -= half;
        return (1 << (m - 1)) + idx;
    }
}

// ngad sturcture bach nsayb dakchi b tari9a mgada

#include <stdint.h>

typedef struct {
    uint8_t id;        /* component identifier (1=Y,2=Cb,3=Cr) */
    uint8_t h_samp;    /* horizontal sampling factor H  (1–4)     */
    uint8_t v_samp;    /* vertical   sampling factor V  (1–4)     */
    uint8_t qt_idx;    /* quant-table index (0–3)                 */
    uint8_t dc_idx;    /* Huffman DC table index (0–3)            */
    uint8_t ac_idx;    /* Huffman AC table index (0–3)            */
} ComponentInfo;



void extract_mcu_block(char* filename){
    /* Génère un fichier contenant uniquement le contenu de tout les blocs de l'image JPEG */
    FILE* file = fopen(filename,"rb");
    FILE* new_file = fopen("mcu_hex.txt","w");
    uint16_t** resultats = size_picture(filename);
    uint16_t N = *(resultats[2]);
    uint16_t* composantes = resultats[1];
    uint16_t* qt_id =  resultats[3];
    int16_t** quant_tables = extract_quant_table(filename);

    ComponentInfo comp[4] = {0};
    for (uint8_t i = 0; i < N; ++i) {
        comp[i].id      = i+1;
        comp[i].v_samp  = composantes[2*i];
        comp[i].h_samp  = composantes[2*i+1];
        comp[i].qt_idx  = qt_id[i]; 
        comp[i].dc_idx  = 0; 
        comp[i].ac_idx  = 0;
    }

    uint8_t comp_count = 0;
    int curr_b;


    while ((curr_b = fgetc(file)) != EOF){
        if (curr_b == 0xFF){
            curr_b = fgetc(file);
            if (curr_b == 0xDA){
                uint16_t taille = (fgetc(file) <<8) + fgetc(file);
                /* taille -2 pour compenser les 2 octets de taille*/
                uint8_t nbr_compo = fgetc(file);
                for (int i=0; i<nbr_compo; i++){
                    comp[i].id = fgetc(file);  
                    uint8_t ID = comp[i].id;
                    for (uint8_t j = 0; j < N; ++j) {
                        if (comp[j].id == ID) {
                            uint8_t hv = fgetc(file);
                            comp[j].dc_idx = hv >> 4;
                            comp[j].ac_idx = hv & 0x0F;
                            break;
                        }
                    
                    }

                    comp[i].qt_idx = qt_id[i];
            
                }
                fgetc(file); fgetc(file); fgetc(file);
                /* tant qu'on est pas encore arrive 
                a la fin de l'image */


                /* AMELIORATION: TRAITEMENT CAS BYTE STUFFING */
                curr_b = fgetc(file);
                while (true){
                    uint8_t prec = curr_b;
                    curr_b= fgetc(file);
                    if (prec == 0xFF){
                        if (curr_b == 0x00){
                            fprintf(new_file, "FF ");
                            curr_b = fgetc(file);
                        }
                        else{
                            break;
                        }
                    }
                    else {
                        fprintf(new_file,"%02X ", prec);
                    }
                    
                }
                
                fprintf(new_file,"\n");
                break;
            }
        }
    }
    fclose(file);
    fclose(new_file);

}

void hex_to_bin(char *filename)
{   
    extract_mcu_block(filename);
    FILE *in  = fopen("mcu_hex.txt", "r");
    FILE *out = fopen("mcu_bin.txt", "w");
    if (!in || !out) {
        perror("hex_to_bin fopen");
        if (in)  fclose(in);
        if (out) fclose(out);
        return;
    }

    unsigned int byte;                 /* %2x lit dans un unsigned int */
    while (fscanf(in, " %2x", &byte) == 1) {  /* ignore blancs, lit deux hex */
        for (int i = 7; i >= 0; --i)
            fputc( (byte >> i & 1) + '0', out );
        //fprintf(out, " ");
    }
    fputc('\n', out);

    fclose(in);
    fclose(out);
}

int16_t** decode_mcu_block(table_de_huffman table_dc, 
    table_de_huffman table_ac, char* filename, uint16_t* taille) {
    
    hex_to_bin(filename);
    int nombre_blocks = (int) (taille[0]*taille[1])/64.0;
    int16_t** mcu_blocks = malloc(nombre_blocks*sizeof(int16_t*));
    for (int j = 0; j < nombre_blocks; j++){
        mcu_blocks[j] = malloc(64 * sizeof(int16_t)); 
    }
    FILE* file = fopen("mcu_bin.txt", "r");

    // Récupération des infos
    int len_dc = table_dc.len;
    huffman_code* huff_dc = table_dc.huff_tab;
    int len_ac = table_ac.len;
    huffman_code* huff_ac = table_ac.huff_tab;

    int16_t prev_dc = 0;

    for (int p = 0; p < nombre_blocks; p++ ){
        
        char seq_cour[17] = {0};
        uint8_t idx = 0;
        uint8_t magnitude;
        uint16_t indice;
        bool code_found = false;
        int curr_b;

        while ((curr_b = get_next_bit(file)) != EOF) {
            if (idx == MAX_HUFF_LEN) {
                fprintf(stderr,
                    "Error: Huffman DC code longer than %d bits – bad JPEG data\n",
                    MAX_HUFF_LEN);          /* clean-up and leave the function */
                goto abort_file;
            }
            int b = curr_b - '0';
            seq_cour[idx] = curr_b;
            idx++;

            for (int i = 0; i < len_dc; i++) {
                uint8_t len = huff_dc[i].longueur;
                /* si la taille de la sequence courante n'est pas egale
                a celle du code de huff, on passe a l'iteration suivante,
                car on est sur que ca correpond pas a un code*/
                if (idx != len) continue;

                /*on tronque le code de huffman courant*/
                char code_tmp[17];
                for (int j = 0; j < len; j++) {
                    code_tmp[j] = huff_dc[i].code[15 - (len - 1 - j)];
                }
                code_tmp[len] = '\0';

                if (strncmp(seq_cour, code_tmp, len) == 0) {
                    magnitude = huff_dc[i].symbole;
                    indice = 0;
                    for (int j = 0; j < magnitude; j++) {
                        b = get_next_bit(file) - '0';
                        indice = (indice << 1) | b;
                    }
                    int16_t dc_diff = magn_indice_to_coeff(magnitude, indice);
                    prev_dc += dc_diff;             /* DC absolu courant   */
                    mcu_blocks[p][0] = prev_dc;  
                    
                    code_found = true;
                    break;
                }
            }
            if (code_found) break;

            if (idx >= MAX_HUFF_LEN/2 && !code_found) {
                // Garder les derniers bits et décaler
                for (int i = 1; i < idx; i++) {
                    seq_cour[i-1] = seq_cour[i];
                }
                idx--;
            }
        }
        
        // --- Décodage AC ---
        int compt = 1;
        idx = 0;
        memset(seq_cour, 0, sizeof(seq_cour));
        code_found = false;

        while (compt < 64 && (curr_b = get_next_bit(file)) != EOF) {
            if (idx == MAX_HUFF_LEN) {
            fprintf(stderr,
                "Error: Huffman AC code longer than %d bits – bad JPEG data\n",
                MAX_HUFF_LEN);
            goto abort_file;
        }
            int b = curr_b - '0';
            seq_cour[idx] = curr_b;
            idx++;
            for (int i = 0; i < len_ac; i++) {
                uint8_t len = huff_ac[i].longueur;
                if (idx != len) continue;

                char code_tmp[17];
                for (int j = 0; j < len; j++) {
                    code_tmp[j] = huff_ac[i].code[15 - (len - 1 - j)];
                }
                code_tmp[len] = '\0';

                if (strncmp(seq_cour, code_tmp, len) == 0) {
                    // Vérifier si c'est EOB
                    if (huff_ac[i].symbole == 0x00) {
                        while (compt < 64){ 
                            mcu_blocks[p][compt] = 0;
                            compt++;
                        }
                        break;
                    }

                    uint8_t coeff_zero = (huff_ac[i].symbole >> 4) & 0x0F;
                    magnitude = huff_ac[i].symbole & 0x0F;
                    indice = 0;

                    for (int j = 0; j < magnitude; j++) {
                        b = get_next_bit(file) - '0';
                        indice = (indice << 1) | b;
                    }

                    for (int j = 0; j < coeff_zero && compt < 64; j++) {
                        mcu_blocks[p][compt++] = 0;
                    }
                    mcu_blocks[p][compt] = magn_indice_to_coeff(magnitude, indice);
                    compt++;
                    /* on reinitialise seq_cour a 0 pour 
                    construire le code suivant*/
                    idx = 0;
                    memset(seq_cour, 0, sizeof(seq_cour));
                    break;
                }
            }
            if (!code_found && idx >= MAX_HUFF_LEN/2) {
                // Technique de décalage pour essayer de récupérer
                for (int i = 1; i < idx; i++) {
                    seq_cour[i-1] = seq_cour[i];
                }
                idx--;
            }
        }
    }

    


    fclose(file);
    return mcu_blocks;


    abort_file:
        fclose(file);
        for (int i = 0; i < nombre_blocks; ++i) 
            free(mcu_blocks[i]);
        free(mcu_blocks);
        return NULL;
}



// int main(int argc, char **argv){
//     uint16_t** resultats = size_picture(argv[1]);
//     uint16_t* taille = resultats[0];

//     table_de_huffman* table = arbre_huffman_V2(argv[1]);

//     table_de_huffman table_dc = table[0];
//     table_de_huffman table_ac = table[1];
//     int16_t** coeff = decode_mcu_block(table_dc,table_ac,argv[1],taille);

//     for (int i = 0; i < (taille[0]*taille[1])/64.0; i++){
//         for (int j = 0; j < 64; j++){
//             printf("%02X ", coeff[i][j]);
//         }
//         printf("\n");
//     }

//     return 0;
// }