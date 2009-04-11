void huffman_init_decode_tree(int arg_0, int first_index, int pTableLo, int pTableHi, int table_size);
void huffman_init_word_decode_tree(int arg_0, int insert_index, int table_lo_ptr, int table_hi_prt, int size);
void AbortReportGuruMeditation(int pData, int arg_4, int arg_8);
int CAErrorMessageBox(int arg_0, int arg_4, int* arg_8);
int init_or_decode_huffman(int arg_0, int arg_4, int huffman_word_str_ptr1);

int init_or_decode_huffman(int arg_0, int arg_4, int huffman_word_str_ptr1)
{
    int huffman_table_size_;
    int dst_table_ptr;

    arg_4 = &aControl_init_c;
    huffman_table_size_ = ((*(BYTE*)huffman_code_check << 8) + *(BYTE*)(huffman_code_check + 1)) & 1023;
    if (huffman_table_size_ > 768)
    {
        CAErrorMessageBox(arg_0, &arg_4 [ 4], 134, arg_4);
        AbortReportGuruMeditation(arg_0, arg_4 [ 16], CAErrorMessageBox(arg_0, &arg_4 [ 4], 134, arg_4));
    }
    huffman_word_str_ptr1 = huffman_word_dictionary;
    huffman_word_str_ptr2 = huffman_word_str_ptr1;
    dst_table_ptr = huffman_word_dictionary_table;
    *dst_table_ptr = huffman_word_str_ptr1;
    dst_index = 1;
    if (huffman_table_size_ > 1)
    {
        do {
            if (*(BYTE*)huffman_word_str_ptr2)
            {
                do {
                    huffman_word_str_ptr2 = huffman_word_str_ptr2 + 1;
                } while (*(BYTE*)huffman_word_str_ptr2); /* end of loop */
            }
            huffman_word_str_ptr2 = huffman_word_str_ptr2 + 1;
            *dst_table_ptr [ dst_index] = huffman_word_str_ptr2;
            dst_index = dst_index + 1;
        } while (huffman_table_size_ > dst_index); /* end of loop */
    }
    huffman_init_word_decode_tree(arg_0, 0, huffman_word_str_ptr1, dst_table_ptr, huffman_table_size_);
    tmp_huffman_table_size_ = huffman_table_size_;
    huffman_table_size_ = ((*(BYTE*)huffman_table_size << 8) + *(BYTE*)(huffman_table_size + 1)) & 1023;
    if (huffman_table_size_ > 1023)
    {
        CAErrorMessageBox(arg_0, &arg_4 [ 4], 155, arg_4);
        AbortReportGuruMeditation(arg_0, arg_4 [ 16], CAErrorMessageBox(arg_0, &arg_4 [ 4], 155, arg_4));
    }
    arg_4 = huffman_table_size + 2;
    return huffman_init_decode_tree(arg_0, 0, arg_4, (WORD*)&arg_4 [ huffman_table_size_], huffman_table_size_);
}

int CAErrorMessageBox(int arg_0, int arg_4, int* arg_8)
{
    int var_C;
    int var_8;
    int var_4;

    var_8 = off_4019EA00;
    var_4 = !(var_8 - &sub_4014CF24);
    if (var_4)
    {
        callDriverFn3(arg_0, dword_40206150);
        if (sub_400F3FF7(arg_0, callDriverFn3(arg_0, dword_40206150)))
        {
            sub_400F4007(arg_0, &loc_400F3DD3, 0);
        }
        dword_40206158 = sub_400B3CF4(arg_0);
    }
    else
    {
        callDriverFn3(arg_0, dword_40206154);
        if (sub_400F3FF7(arg_0, callDriverFn3(arg_0, dword_40206154)))
        {
            sub_400F4007(arg_0, &loc_400F3DD3, 1);
        }
        dword_4020615C = sub_400B3CF4(arg_0);
    }
    var_C = &arg_8;
    var_10 = sub_400F399C(arg_0, var_8, arg_4, &var_C);
    var_C = -256;
    if (var_4)
    {
        dword_40206158 = 0;
        callDriverFn2(arg_0, dword_40206150);
        if (sub_400F3FF7(arg_0, callDriverFn2(arg_0, dword_40206150)))
        {
            sub_400F403E(arg_0);
            return var_10;
        }
    }
    else
    {
        dword_4020615C = 0;
        callDriverFn2(arg_0, dword_40206154);
        if (sub_400F3FF7(arg_0, callDriverFn2(arg_0, dword_40206154)))
        {
            sub_400F403E(arg_0);
        }
    }
    return var_10;
}

void AbortReportGuruMeditation(int pData, int arg_4, int arg_8)
{

    arg_8 = &aAbortReportGuruMedi;
    sub_40194238(pData, dword_4019DE08, arg_8, arg_4);
    sub_400B3F3C(pData, arg_8 [ 12], dword_4019DE08);
    for (;;)
    {
        GetTickTimer(pData, arg_8 [ 13], sub_400B3F3C(pData, arg_8 [ 12], dword_4019DE08));
    } /* end of loop */
}

void huffman_init_word_decode_tree(int arg_0, int insert_index, int table_lo_ptr, int table_hi_prt, int size)
{

    if ((insert_index > -1) && (insert_index <= 3))
    {
        *huffman_word_lookup_lo [ insert_index] = table_lo_ptr;
        *huffman_word_lookup_hi [ insert_index] = table_hi_prt;
        if (!table_hi_prt | !table_lo_ptr)
        {
            *huffman_lookup_2 [ insert_index] = 0;
        }
        else
        {
            *huffman_lookup_2 [ insert_index] = size;
            return;
        }
    }
    return;
}

void huffman_init_decode_tree(int arg_0, int first_index, int pTableLo, int pTableHi, int table_size)
{
    int* var_8;
    int index;

    var_8 = 0;
    if ((first_index > -1) && (first_index <= 3))
    {
        *huffman_lo_bit_branch [ first_index] = pTableLo;
        *huffman_hi_bit_branch [ first_index] = pTableHi;
        if (!pTableLo | !pTableHi)
        {
            *huffman_valid_start_code [ first_index] = 0;
        }
        else
        {
            *huffman_valid_start_code [ first_index] = table_size;
        }
        *(WORD*)&var_8 = 4080;
        if (*(BYTE*)&var_8 == 240)
        {
            index = 0;
            if (table_size > 0)
            {
                do {
                    first_index = (WORD*)&pTableLo [ index];
                    *(WORD*)&var_8 = *(short*)first_index & 0xffff;
                    *(WORD*)first_index = (((*(short*)first_index & 0xffff) >> 8) & 0xff) + ((*(WORD*)&var_8 & 0xff) << 8);
                    first_index = (WORD*)&pTableHi [ index];
                    *(WORD*)&var_8 = *(short*)first_index & 0xffff;
                    *(WORD*)first_index = (((*(short*)first_index & 0xffff) >> 8) & 0xff) + ((*(WORD*)&var_8 & 0xff) << 8);
                    index = index + 1;
                } while (table_size > index); /* end of loop */
            }
        }
        return;
    }
    return;
}
