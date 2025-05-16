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
    } while (c != '0' && c != '1');     
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






void decode_one_block(
        FILE *bitstream,          //flux  binaire genere
        int16_t coef[64],         // tableau destination 8×8             
        int16_t *prev_dc,         // valeur DC précédente de la composante
        table_de_huffman dc_tab,  // table Huffman DC
        table_de_huffman ac_tab)  // table Huffman AC
{

    memset(coef, 0, 64 * sizeof(int16_t));

  //DC
    char seq_cour[17] = {0};
    uint8_t idx = 0;
    int curr_b;


    while ((curr_b = get_next_bit(bitstream)) != EOF) {
        seq_cour[idx++] = curr_b;
        for (int i = 0; i < dc_tab.len; ++i) {
            /* si la taille de la sequence courante n'est pas egale
                a celle du code de huff, on passe a l'iteration suivante,
                car on est sur que ca correpond pas a un code*/
            if (idx != dc_tab.huff_tab[i].longueur) continue;



            if (!strncmp(seq_cour,
                         dc_tab.huff_tab[i].code + 16 - idx, /* k7z ghir lpointeur fin bghiti t comparer no need dir dik for loop*/
                         idx))
            {
                uint8_t magnitude = dc_tab.huff_tab[i].symbole;
                uint16_t indice   = 0;

                for (uint8_t j = 0; j < magnitude; j++)
                    indice = (indice << 1) | (get_next_bit(bitstream) - '0');

                int16_t dc = *prev_dc + magn_indice_to_coeff(magnitude, indice);
                *prev_dc   = dc;    /* DC absolu courant   */
                coef[0]    = dc;
                idx = 0;                              
                memset(seq_cour, 0, sizeof seq_cour);
                goto decode_ac;
            }
        }
        if (idx == MAX_HUFF_LEN) {
            fprintf(stderr, "DC code trop long – JPEG corrompu\n");
            return;
        }
    }

decode_ac:
    
    int compt = 1;                                     
    while (compt < 64 && (curr_b = get_next_bit(bitstream)) != EOF) {
        seq_cour[idx] = curr_b;
        idx++;

        for (int i = 0; i < ac_tab.len; ++i) {
            
            if (idx != ac_tab.huff_tab[i].longueur) continue;

            if (!strncmp(seq_cour,
                         ac_tab.huff_tab[i].code + 16 - idx,
                         idx))
            {
                uint8_t symbol = ac_tab.huff_tab[i].symbole;
                if (symbol == 0x00) {        
                    return;
                }

                uint8_t coeff_zero        = symbol >> 4;   
                uint8_t magnitude  = symbol & 0x0F; 

                uint16_t indice = 0;
                for (uint8_t k = 0; k < magnitude; ++k)
                    indice = (indice << 1) | (get_next_bit(bitstream) - '0');


                while (coeff_zero-- && compt < 64) coef[compt++] = 0;

                /* écrire le coefficient non-nul */
                if (compt < 64)
                    coef[compt++] = magn_indice_to_coeff(magnitude, indice);

                idx = 0;
                memset(seq_cour, 0, sizeof seq_cour);
                break;               
            }
        }
        if (idx == MAX_HUFF_LEN) {
            fprintf(stderr, "AC code trop long – JPEG corrompu\n");
            return;
        }
    }
}

uint8_t init_component_info(const char *jpeg_path, ComponentInfo comp[3])
{
    /* 1)  Récupérer les métadonnées de l’image -------------------- */
    uint16_t **infos      = size_picture((char *)jpeg_path);
    uint16_t  N           = *infos[2];        /* nombre de composantes */
    uint16_t *composantes =  infos[1];        /* tableau [V1,H1,V2,H2…] */
    uint16_t *qt_id       =  infos[3];        /* index des tables Q     */

    /* 2)  Remplir comp[] ------------------------------------------ */
    for (uint8_t i = 0; i < N && i < 3; ++i) {
        comp[i].id      = i + 1;                     /* 1 = Y, 2 = Cb, 3 = Cr */
        comp[i].h_samp  = composantes[2*i + 1];      /* H_i du SOF           */
        comp[i].v_samp  = composantes[2*i];          /* V_i du SOF           */
        comp[i].qt_idx  = qt_id[i];                  /* table de quantif     */
        comp[i].dc_idx  = 0;                         /* les valeurs réelles  */
        comp[i].ac_idx  = 0;                         /* seront fixées après  */
    }
    return (uint8_t)N;
}


int16_t*** decode_mcu_blocks_444(
        table_de_huffman *tables_dc, table_de_huffman *tables_ac,
        char *filename,
        uint16_t *taille,
        ComponentInfo *comp,
        uint8_t nb_comp)
{
    


    uint8_t H_Y = comp[0].h_samp;
    uint8_t V_Y = comp[0].v_samp;


    int mcu_w = (taille[0] + 8*H_Y - 1) / (8*H_Y);
    int mcu_h = (taille[1] + 8*V_Y - 1) / (8*V_Y);
    int total_mcu = mcu_w * mcu_h;

    int16_t ***components = malloc(nb_comp * sizeof *components);
    for (uint8_t c = 0; c < nb_comp; ++c) {
        int nb_blocs = total_mcu * comp[c].h_samp * comp[c].v_samp;
        components[c] = malloc(nb_blocs * sizeof **components);
        for (int b = 0; b < nb_blocs; ++b)
            components[c][b] = calloc(64, sizeof ***components);
    }

    
    hex_to_bin(filename);            
    FILE *file = fopen("mcu_bin.txt", "r");
    if (!file) { perror("fopen mcu_bin"); return NULL; }

    int16_t prev_dc[3] = {0};




    for (int my = 0; my < mcu_h; ++my) {
        for (int mx = 0; mx < mcu_w; ++mx) {
            for (uint8_t c = 0; c < nb_comp; ++c) {
                for (uint8_t v = 0; v < comp[c].v_samp; ++v) {
                    for (uint8_t h = 0; h < comp[c].h_samp; ++h) {

                        int bloc =
                            (my * comp[c].v_samp + v) * mcu_w * comp[c].h_samp +
                            (mx * comp[c].h_samp + h);

                        decode_one_block(file,
                                         components[c][bloc],
                                         &prev_dc[c],
                                         tables_dc[comp[c].dc_idx],
                                         tables_ac[comp[c].ac_idx]);
                    }
                }
            }
        }
    }
    fclose(file);
    return components;
}








int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <jpeg_file>\n", argv[0]);
        return 1;
    }


    uint16_t** resultats = size_picture(argv[1]);
    uint16_t* taille = resultats[0];
    uint16_t N = *(resultats[2]); 
    uint16_t* composantes = resultats[1];
    uint16_t* qt_id = resultats[3];


    uint16_t h_reel = taille[0];
    uint16_t l_reel = taille[1];

    uint16_t* taille_reel = malloc(2*sizeof(uint16_t));
    taille_reel[0] = h_reel;
    taille_reel[1] = l_reel;

    if (taille[1]%8 != 0 && taille[0]%8 != 0){
        taille[1] = 8*((l_reel/8) +1);
        taille[0] = 8*((h_reel/8) +1);
    }
    
    else if (taille[1]%8 !=0){
        taille[1] = 8*((l_reel/8) +1);
        
    }
    else if (taille[0]%8 !=0){
        taille[0] = 8*((h_reel/8) +1);
    }
    else{
        printf("Pas besoin de troncature!\n");
    }

    ComponentInfo comp[3] = {0};  // Maximum 3 composantes (Y, Cb, Cr)
    
    // on remplit les structures ComponentInfo
    for (uint8_t i = 0; i < N; i++) {
        comp[i].id = i + 1;            // id (1=y, 2=cb, 3=cr)
        comp[i].h_samp = composantes[2*i+1];            // (on suppose 1 pour simplifier)  apres on peux just les remplires par composantes[2*i+1/2*i]
        comp[i].v_samp = composantes[2*i];          
        comp[i].qt_idx = qt_id[i];    
    }
    
    // extraction des tables de Huffman
    table_de_huffman** tables = arbre_huffman_V2(argv[1]);
    

    table_de_huffman tables_dc[2] = {tables[0][0], tables[0][1]};  // Tables DC
    table_de_huffman tables_ac[2] = {tables[1][0], tables[1][1]};  // Tables AC
    
  
    comp[0].dc_idx = 0;  // Y utilise la table DC 0
    comp[0].ac_idx = 0;  // Y utilise la table AC 0
    comp[1].dc_idx = 1;  // Cb utilise la table DC 1
    comp[1].ac_idx = 1;  // Cb utilise la table AC 1
    comp[2].dc_idx = 1;  // Cr utilise la table DC 1
    comp[2].ac_idx = 1;  // Cr utilise la table AC 1
    
    // decodage des MCU pour obtenir les composantes Y, Cb, Cr
    int16_t*** components = decode_mcu_blocks_444(tables_dc, tables_ac, argv[1], taille, comp, N);
    
    if (components == NULL) {
        fprintf(stderr, "Erreur lors du décodage des MCU\n");
        return 1;
    }
    
    // affichage des résultats pour vérification
    printf("Composantes de l'image décodées :\n");
    
    // Calculer le nombre de blocs par composante
    int width_in_blocks = (taille[0] + 7) / 8;
    int height_in_blocks = (taille[1] + 7) / 8;
    int total_blocks = width_in_blocks * height_in_blocks;
    
    // Pour chaque composante
    for (uint8_t c = 0; c < N; c++) {
        const char* comp_name;
        switch (c) {
            case 0: comp_name = "Y"; break;
            case 1: comp_name = "Cb"; break;
            case 2: comp_name = "Cr"; break;
            default: comp_name = "je ne sai pas c'est quoi cette couleur de merde"; break;
        }
        
        printf("Composante %s :\n", comp_name);
        
        // afficher les 5 prem blocks
        //int blocks_to_show = (total_blocks < 5) ? total_blocks : 10;
        int blocks_to_show = total_blocks;
        for (int b = 0; b < blocks_to_show; b++) {
            printf("  Bloc %d :\n", b);
            for (int i = 0; i < 8; i++) {
                printf("    ");
                for (int j = 0; j < 8; j++) {
                    int idx = i * 8 + j;
                    printf("%4d ", components[c][b][idx]);
                }
                printf("\n");
            }
        }
    }
    
    // lib de la mémoire
    for (uint8_t c = 0; c < N; c++) {
        for (int j = 0; j < total_blocks; j++) {
            free(components[c][j]);
        }
        free(components[c]);
    }
    free(components);
    
    
    return 0;
}


