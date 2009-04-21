   if ( from[0] == 0x1f ) {
       char *temp = freesat_huffman_decode(from, len);
       if (temp ) {
           len = strlen(temp);
           len = len < size - 1 ? len : size - 1;
           strncpy(buffer, temp, len);
           buffer[len] = 0;
           free(temp);
           return;
       }
   }

   if ( from[0] == 0x1f ) {
       temp = ( unsigned char *)freesat_huffman_decode(from, len);
       from = temp;
   }

   if (temp) free(temp);
