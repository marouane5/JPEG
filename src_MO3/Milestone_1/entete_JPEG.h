#include <stdint.h>

extern uint32_t size_picture(char* path);

extern uint32_t extract_largeur(char* path);

extern uint32_t* extract_facteur_echant(char* path);

extern float* extract_tab_quant(char* path);

extern float* extract_table_huff(char* path);
