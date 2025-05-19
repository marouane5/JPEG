#ifndef ENTETE_JPEG_H    // Garde d'inclusion : évite les inclusions multiples
#define ENTETE_JPEG_H

#include <stdint.h>

typedef struct huffman_code{
    uint8_t symbole;
    char code[17];
    uint8_t longueur;

}huffman_code;

typedef struct table_de_huffman{
    int len;
    huffman_code* huff_tab;
}table_de_huffman;

typedef struct {
    uint8_t id;        /* component identifier (1=Y,2=Cb,3=Cr) */
    uint8_t h_samp;    /* horizontal sampling factor H  (1–4)     */
    uint8_t v_samp;    /* vertical   sampling factor V  (1–4)     */
    uint8_t qt_idx;    /* quant-table index (0–3)                 */
    uint8_t dc_idx;    /* Huffman DC table index (0–3)            */
    uint8_t ac_idx;    /* Huffman AC table index (0–3)            */
} ComponentInfo;



uint16_t** extract_image_info(char* path);
    /* prend en argument l'image jpeg et renvoie un tableau
    de 4 cases: 
    * case 1: pointe vers un tableau de 2 cases: hauteur, largeur
    * case 2: pointe vers un tableau de 2*N cases: les facteurs d'ech
    (N: nbr de composantes: N=1 ou N=3)
    * case 3: double pointeur vers N 
    * case 4: pointe vers un tableau de N cases: contient l'indice 
    de la table de quantif qui correspond a chaque composante
    */
void init_component_info(const char *jpeg_path, ComponentInfo comp[3], uint8_t** huff_idx_tables);
int16_t** extract_quant_tables(char* path);
    /* renvoie un tableau de 4 cases et chaque case pointe
    vers la table de quantification associee ou NULL si celle
    ci n'existe pas */
uint8_t**** extract_huffman_info(char* path);
    /* 
    * renvoie un tableau de 2 cases pour DC et AC
    * chaque case pointe vers un tableau de 4 cases 
    (4 tableaux par composante au maximum)
    * chaque case de ce dernier tableau pointe vers un 
    tableau de 16 cases (chaque case i correpond a une longueur de i+1)
    * chaque case i de ce dernier tableau pointe vers un tableau 
    de n+1 elements (n nombre de symboles dont le code est de 
    longueur i+1): le premier element de ce tableau fournit
    la valeur de n et les cases restantes indiquent les symboles 
    dont le code est de longueur i+1
    */
void dec_to_bin(uint16_t n, char* buffer);
table_de_huffman** construction_arbre_huffman(char* path);
    /* renvoie un tableau de deux cases pour DC et AC
    * chaque case pointe vers un tableu de 4 cases
    * chaque case de ce dernier tableau est une 
    structure table_de_huffman (qui se construit en utilisant
    les informations fournies par la fonction precedante)
    */
uint8_t** extract_huff_idx(char* filename);
    /*
    Renvoie une table contenant pour Y, Cb et Cr les indices de
    table de Huffman utilisées pour DC et AC
    */




#endif