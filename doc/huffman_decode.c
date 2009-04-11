int huffman_func_decode(int arg_0, int src_ptr, int src_len, int dest_ptr, int result_code_dest_len);

int huffman_func_decode(int arg_0, int src_ptr, int src_len, int dest_ptr, int result_code_dest_len)
{
    int bit_mask;
    int var_1C;
    int current_byte_to_decode;
    int huffman_lo_bit_branch_ptr;
    int huffman_hi_bit_branch_ptr;
    int dest_len1;
    int dest_count;
    int src_index;

    if (result_code_dest_len && dest_ptr)
    {
        if (src_len && src_ptr)
        {
            var_1C = *(BYTE*)src_ptr >> 6;
            if ((var_1C < 4) && *huffman_valid_start_code [ var_1C])
            {
                huffman_hi_bit_branch_ptr = *huffman_hi_bit_branch [ var_1C];
                huffman_lo_bit_branch_ptr = *huffman_lo_bit_branch [ var_1C];
                current_byte_to_decode = *(BYTE*)src_ptr;
                bit_mask = 32;
                src_index = 0;
                dest_count = 0;
                dest_len1 = result_code_dest_len + -1;
                result_code_dest_len = 0;
                if (dest_len1 > 0)
                {
                    do {
                        if (current_byte_to_decode & bit_mask)
                        {
                            result_code_dest_len = *(short*)(WORD*)&huffman_hi_bit_branch_ptr [ result_code_dest_len];
                        }
                        else
                        {
                            result_code_dest_len = *(short*)(WORD*)&huffman_lo_bit_branch_ptr [ result_code_dest_len];
                        }
                        if (result_code_dest_len < 0)
                        {
                            result_code_dest_len = -1 - result_code_dest_len;
                            if (result_code_dest_len < 256)
                            {
                                *(BYTE*)dest_ptr = result_code_dest_len;
                                dest_ptr = dest_ptr + 1;
                                dest_count = dest_count + 1;
                            }
                            else
                            {
                                result_code_dest_len = **huffman_word_lookup_hi [ var_1C] [ &result_code_dest_len [ -64]];
                                if ((dest_len1 > dest_count) && *(SBYTE*)result_code_dest_len)
                                {
loc_40130C11:
                                    *(BYTE*)dest_ptr = *(SBYTE*)result_code_dest_len;
                                    dest_ptr = dest_ptr + 1;
                                    result_code_dest_len = result_code_dest_len + 1;
                                    dest_count = dest_count + 1;
                                    while ((dest_len1 <= dest_count) || !*(SBYTE*)result_code_dest_len)
                                    {
                                        goto loc_40130C11;
                                        *(BYTE*)dest_ptr = *(SBYTE*)result_code_dest_len;
                                        dest_ptr = dest_ptr + 1;
                                        result_code_dest_len = result_code_dest_len + 1;
                                        dest_count = dest_count + 1;
                                    } /* end of while */
                                }
                            }
                            result_code_dest_len = 0;
                        }
                        bit_mask = (((bit_mask < 0) + bit_mask) >> 1) & 0xff;
                        if (!bit_mask)
                        {
                            src_index = src_index + 1;
                            src_ptr = src_ptr + 1;
                            if (src_len > src_index)
                            {
                                current_byte_to_decode = *(BYTE*)src_ptr;
                                bit_mask = 128;
                            }
                            else
                            {
loc_40130C57:
                                *(BYTE*)dest_ptr = 0;
                                return dest_count + 1;
                            }
                        }
                    } while (dest_len1 > dest_count); /* end of loop */
                    goto loc_40130C57;
                }
                else
                    goto loc_40130C57;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}
