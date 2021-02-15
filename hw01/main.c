#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>


#define TABLE_LEN 256
#define STR_LEN 64

typedef struct code_table_ {
    char name[STR_LEN];
    int ucode[TABLE_LEN];
} code_table;




void read_code_table(code_table *ct)
{
    // printf("%s() begin\n", __func__);

    char fname[STR_LEN];
    FILE *f;
    char str[STR_LEN];
    char ccode[3];
    char ucode[5];
    int i;
        

    // printf("Table name: %s\n", ct->name);

    strcpy(fname, ct->name);
    strcat(fname, ".uct");

    f = fopen(fname, "r");

    if(!f) {
        fprintf(stderr, "Error read code table file %s!!!\n", fname);
        exit(1);
    }
 
    while(fgets(str, STR_LEN, f)) {
        //  printf("%s", str);

        // Ignore comments
        if(str[0] != '#' && str[0] == '0') {
            memset(ccode, 0, 3);
            memset(ucode, 0, 5);

            // Get index for table
            if(str[0] == '0' && str[1] == 'x') {
                strncpy(ccode, str+2, 2);
                i = (int)strtol(ccode, NULL, 16);
            }

            // Add unicode code to table
            if(str[5] == '0' && str[6] == 'x') {
                strncpy(ucode, str+7, 4);
                ct->ucode[i] = (int)strtol(ucode, NULL, 16);
            }
        }
    }

    fclose(f);
}

void encode_to_utf8(code_table *ct, const char *in_file, const char *out_file)
{
    FILE *inf, *outf;
    uint8_t b, b1, b2, b3, b4;
    int ucode;
    

    inf = fopen(in_file, "rb");
    if(!inf) {
        fprintf(stderr, "Error read input file %s!!!\n", in_file);
        exit(1);
    }

    outf = fopen(out_file, "wb");
    if(!outf) {
        fprintf(stderr, "Error create output file %s!!!\n", out_file);
        exit(1);
    }

    while(fread(&b, sizeof(b), 1, inf)) {
        // printf("%x ", b);
        ucode = ct->ucode[b];

        // Ignore undefind unicode values
        if(b && !ucode)  
            continue;

        // Encode to utf8
        if(ucode < 128) {
            fwrite(&ucode, sizeof(b), 1, outf);
        }
        else if (ucode < 2048) {
            b1 = (ucode & 0x3f) | 0x80;
            b2 = (ucode >> 6) | 0xc0;
            fwrite(&b2, sizeof(b), 1, outf);
            fwrite(&b1, sizeof(b), 1, outf);
        }
        else if (ucode < 65536) {
            b1 = (ucode & 0x3f) | 0x80;
            b2 = ((ucode >> 6) & 0x3f) | 0x80;
            b3 = (ucode >> 12) | 0xe0;
            fwrite(&b3, sizeof(b), 1, outf);
            fwrite(&b2, sizeof(b), 1, outf);
            fwrite(&b1, sizeof(b), 1, outf);
        }
        else if (ucode < 2097152) {
            b1 = (ucode & 0x3f) | 0x80;
            b2 = ((ucode >> 6) & 0x3f) | 0x80;
            b3 = ((ucode >> 12) & 0x3f) | 0x80;
            b4 = (ucode >> 18) | 0xf0;
            fwrite(&b4, sizeof(b), 1, outf);
            fwrite(&b3, sizeof(b), 1, outf);
            fwrite(&b2, sizeof(b), 1, outf);
            fwrite(&b1, sizeof(b), 1, outf);
        }
    }

    fclose(inf);
    fclose(outf);
}

int main(int argc, char **argv) {
    //int i;
    static code_table c_tbl;
    
    if (argc == 1) {
        printf("Usage: ./utf8_encoder encoding_name out_file in_file\n");
        exit(0);
    }
    else if(argc != 4) {
        fprintf(stderr, "Error arguments count!!!\n");
        exit(1);
    }

    // for(i=0; i<argc; i++)
	    // printf("%s ", argv[i]);

    strcpy(c_tbl.name, argv[1]);
    read_code_table(&c_tbl);

    // for(i=0; i<TABLE_LEN; i++) {
    //     if(!i || c_tbl.ucode[i])  // Ignore undefind unicode values
    //         printf("ct str%d: %d %X %d %X\n", i, i, i, c_tbl.ucode[i], c_tbl.ucode[i]);
    // }

    encode_to_utf8(&c_tbl, argv[3], argv[2]);

    return 0;
}
