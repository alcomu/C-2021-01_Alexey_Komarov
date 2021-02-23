#include "sign.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 256

int main(int argc, char **argv) {
    FILE *inf;
    uint8_t b;
    int i = 0;
    uint16_t jpeg_sign = 0;
    uint32_t zip_sign = 0;
    int found_sign_count;
    int catalog_count = 0;
    int file_count = 0;
    cfh cf_header;
    char buff[BUFF_LEN];

    if (argc == 1) {
        printf("Usage: ./jpeg_zip_detector infile\n");
        exit(0);
    } else if (argc != 2) {
        fprintf(stderr, "Need infile for analize!!!\n");
        exit(1);
    }

    inf = fopen(argv[1], "rb");
    if (!inf) {
        fprintf(stderr, "Error read input file %s!!!\n", argv[1]);
        exit(1);
    }

    printf("Search jpeg signatures in %s:\n", argv[1]);
    found_sign_count = 0;
    while (fread(&b, sizeof(b), 1, inf)) {
        jpeg_sign = (jpeg_sign << 8) | b;
        i++;

        if (jpeg_sign == JSIGN_SOI) {
            printf("  SOI sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_SOF0) {
            printf("  SOF0 sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_SOF1) {
            printf("  SOF1 sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_SOF2) {
            printf("  SOF2 sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_DHT) {
            printf("  DHT sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGNf_DQT) {
            printf("  DQT sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_DRI) {
            printf("  DRI sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_SOS) {
            printf("  SOS sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_COM) {
            printf("  COM sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if (jpeg_sign == JSIGN_EOI) {
            printf("  EOI sign offset: %lu\n", i - sizeof(jpeg_sign));
            found_sign_count++;
            break;
        } else if ((jpeg_sign & 0xfff0) == JSIGN_RST) {
            printf("  RST%d sign offset: %lu\n", (jpeg_sign & 0x000f), i - sizeof(jpeg_sign));
            found_sign_count++;
        } else if ((jpeg_sign & 0xfff0) == JSIGN_APP) {
            printf("  APP%d sign offset: %lu\n", (jpeg_sign & 0x000f), i - sizeof(jpeg_sign));
            found_sign_count++;
        }
    }
    if (!found_sign_count)
        printf("Not found\n");
    else
        printf("Sign count: %d\n", found_sign_count);


    printf("\n\nSearch zip signatures in %s:\n", argv[1]);
    found_sign_count = 0;
    while (fread(&b, sizeof(b), 1, inf)) {
        i++;
        zip_sign = (zip_sign << 8) | b;

        if (zip_sign == ZSIGN_LFH) {
            printf("  LFH sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        } else if (zip_sign == ZSIGN_AEDH) {
            printf("  AEDH sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        } else if (zip_sign == ZSIGN_CDH) {
            printf("  CDH sign offset: %lu\n", i - sizeof(zip_sign));

            if (fread(&cf_header, sizeof(cf_header), 1, inf)) {
                i += sizeof(cf_header);

                // Prepare buffer
                memset(buff, 0, BUFF_LEN);

                // Read and print filename
                if (fread(&buff, cf_header.name_len, 1, inf)) {
                    i += cf_header.name_len;

                    if (buff[cf_header.name_len - 1] == '/') {
                        catalog_count++;
                        printf("    Catalog name: %s\n", buff);
                    } else {
                        file_count++;
                        printf("    File name: %s\n", buff);
                    }
                }
            }

            found_sign_count++;
        } else if (zip_sign == ZSIGN_DSR) {
            printf("  DSR sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        } else if (zip_sign == ZSIGN_ZIP64_EOCDR) {
            printf("  ZIP64_EOCDR sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        } else if (zip_sign == ZSIGN_ZIP64_EOCDL) {
            printf("  ZIP64_EOCDL sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        } else if (zip_sign == ZSIGN_EOCDR) {
            printf("  EOCDR sign offset: %lu\n", i - sizeof(zip_sign));
            found_sign_count++;
        }
    }
    if (!found_sign_count) {
        printf("Not found\n");
    } else {
        printf("Sign count: %d\n", found_sign_count);
        printf("Catalog count: %d\n", catalog_count);
        printf("File count: %d\n", file_count);
    }

    fclose(inf);

    printf("\n\nFile size: %d bytes\n", i);

    return 0;
}