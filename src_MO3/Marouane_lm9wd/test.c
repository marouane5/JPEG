int16_t*** decode_mcu_block(table_de_huffman* tables_dc, 
    table_de_huffman* tables_ac, char* filename, uint16_t* taille, ComponentInfo* comp, uint8_t nb_comp) {

    /* Calculate MCU dimensions based on sampling factors */
    uint8_t max_h_samp = 0;
    uint8_t max_v_samp = 0;
    for (uint8_t i = 0; i < nb_comp; i++) {
        if (comp[i].h_samp > max_h_samp) max_h_samp = comp[i].h_samp;
        if (comp[i].v_samp > max_v_samp) max_v_samp = comp[i].v_samp;
    }

    /* Calculate number of blocks for each component */
    int width_in_blocks = (taille[0] + 7) / 8;  // Ceiling division by 8
    int height_in_blocks = (taille[1] + 7) / 8; // Ceiling division by 8
    
    int width_in_mcu = (width_in_blocks + max_h_samp - 1) / max_h_samp;
    int height_in_mcu = (height_in_blocks + max_v_samp - 1) / max_v_samp;
    
    int total_mcus = width_in_mcu * height_in_mcu;
    
    /* Allocate memory for the result */
    int16_t*** components = malloc(nb_comp * sizeof(int16_t**));
    for (uint8_t c = 0; c < nb_comp; c++) {
        int blocks_per_comp = total_mcus * comp[c].h_samp * comp[c].v_samp;
        components[c] = malloc(blocks_per_comp * sizeof(int16_t*));
        for (int j = 0; j < blocks_per_comp; j++) {
            components[c][j] = malloc(64 * sizeof(int16_t));
            // Initialize to zero
            memset(components[c][j], 0, 64 * sizeof(int16_t));
        }
    }

    hex_to_bin(filename);
    FILE* file = fopen("mcu_bin.txt", "r");
    if (!file) {
        perror("Failed to open binary data file");
        // Clean up and return NULL
        for (uint8_t c = 0; c < nb_comp; c++) {
            int blocks_per_comp = total_mcus * comp[c].h_samp * comp[c].v_samp;
            for (int j = 0; j < blocks_per_comp; j++) {
                free(components[c][j]);
            }
            free(components[c]);
        }
        free(components);
        return NULL;
    }

    /* Previous DC values for each component */
    int16_t* prev_dc = calloc(nb_comp, sizeof(int16_t));

    /* Process each MCU */
    for (int mcu_idx = 0; mcu_idx < total_mcus; mcu_idx++) {
        /* Process each component in the MCU */
        for (uint8_t c = 0; c < nb_comp; c++) {
            /* Get the Huffman tables for this component */
            table_de_huffman table_dc = tables_dc[comp[c].dc_idx];
            table_de_huffman table_ac = tables_ac[comp[c].ac_idx];
            
            int len_dc = table_dc.len;
            huffman_code* huff_dc = table_dc.huff_tab;
            int len_ac = table_ac.len;
            huffman_code* huff_ac = table_ac.huff_tab;
            
            /* Process h_samp * v_samp blocks for this component */
            for (int v = 0; v < comp[c].v_samp; v++) {
                for (int h = 0; h < comp[c].h_samp; h++) {
                    int block_idx = mcu_idx * (comp[c].h_samp * comp[c].v_samp) + v * comp[c].h_samp + h;
                    
                    /* Decode DC coefficient */
                    char seq_cour[17] = {0};
                    uint8_t idx = 0;
                    uint8_t magnitude;
                    uint16_t indice;
                    bool code_found = false;
                    int curr_b;

                    while ((curr_b = get_next_bit(file)) != EOF) {
                        if (idx == MAX_HUFF_LEN) {
                            fprintf(stderr,
                                "Error: Huffman DC code longer than %d bits - bad JPEG data\n",
                                MAX_HUFF_LEN);
                            goto abort_file;
                        }
                        int b = curr_b - '0';
                        seq_cour[idx] = curr_b;
                        idx++;

                        for (int i = 0; i < len_dc; i++) {
                            uint8_t len = huff_dc[i].longueur;
                            /* Check if current sequence matches a Huffman code */
                            if (idx != len) continue;

                            /*Truncate the current huffman code*/
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
                                prev_dc[c] += dc_diff;             /* Current absolute DC */
                                components[c][block_idx][0] = prev_dc[c];  
                                
                                code_found = true;
                                break;
                            }
                        }
                        if (code_found) break;

                        if (idx >= MAX_HUFF_LEN/2 && !code_found) {
                            // Keep last bits and shift
                            for (int i = 1; i < idx; i++) {
                                seq_cour[i-1] = seq_cour[i];
                            }
                            idx--;
                        }
                    }
                    
                    /* Decode AC coefficients */
                    int compt = 1;
                    idx = 0;
                    memset(seq_cour, 0, sizeof(seq_cour));
                    code_found = false;

                    while (compt < 64 && (curr_b = get_next_bit(file)) != EOF) {
                        if (idx == MAX_HUFF_LEN) {
                            fprintf(stderr,
                                "Error: Huffman AC code longer than %d bits - bad JPEG data\n",
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
                                // Check if it's EOB
                                if (huff_ac[i].symbole == 0x00) {
                                    while (compt < 64){ 
                                        components[c][block_idx][compt] = 0;
                                        compt++;
                                    }
                                    code_found = true;
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
                                    components[c][block_idx][compt++] = 0;
                                }
                                components[c][block_idx][compt] = magn_indice_to_coeff(magnitude, indice);
                                compt++;
                                /* Reset seq_cour to 0 to build the next code */
                                idx = 0;
                                memset(seq_cour, 0, sizeof(seq_cour));
                                code_found = true;
                                break;
                            }
                        }
                        if (code_found) {
                            code_found = false;
                            continue;
                        }
                        
                        if (idx >= MAX_HUFF_LEN/2) {
                            // Shifting technique to try to recover
                            for (int i = 1; i < idx; i++) {
                                seq_cour[i-1] = seq_cour[i];
                            }
                            idx--;
                        }
                    }
                    
                    // Ensure any remaining AC coefficients are set to 0
                    while (compt < 64) {
                        components[c][block_idx][compt++] = 0;
                    }
                }
            }
        }
    }

    free(prev_dc);
    fclose(file);
    return components;

abort_file:
    if (file) fclose(file);
    free(prev_dc);
    for (uint8_t c = 0; c < nb_comp; c++) {
        int blocks_per_comp = total_mcus * comp[c].h_samp * comp[c].v_samp;
        for (int j = 0; j < blocks_per_comp; j++) {
            free(components[c][j]);
        }
        free(components[c]);
    }
    free(components);
    return NULL;
}