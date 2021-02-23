#ifndef SIGN_H
#define SIGN_H

#include <inttypes.h>

// JPEG signatures
#define JSIGN_SOI 0xffd8
#define JSIGN_SOF0 0xffc0
#define JSIGN_SOF1 0xffc1
#define JSIGN_SOF2 0xffc2
#define JSIGN_DHT 0xffc4
#define JSIGNf_DQT 0xffdb
#define JSIGN_DRI 0xffdd
#define JSIGN_SOS 0xffda
#define JSIGN_RST 0xffd0
#define JSIGN_APP 0xffe0
#define JSIGN_COM 0xfffe
#define JSIGN_EOI 0xffd9

// ZIP signatures
#define ZSIGN_LFH 0x504b0304
#define ZSIGN_AEDH 0x504b0608
#define ZSIGN_CDH 0x504b0102
#define ZSIGN_DSR 0x504b0505
#define ZSIGN_ZIP64_EOCDR 0x504b0606
#define ZSIGN_ZIP64_EOCDL 0x504b0607
#define ZSIGN_EOCDR 0x504b0506

/* Central File Header (Central Directory Entry) */
typedef struct cfh_ {
    uint16_t made_by_ver;    /* Version made by. */
    uint16_t extract_ver;    /* Version needed to extract. */
    uint16_t gp_flag;        /* General purpose bit flag. */
    uint16_t method;         /* Compression method. */
    uint16_t mod_time;       /* Modification time. */
    uint16_t mod_date;       /* Modification date. */
    uint32_t crc32;          /* CRC-32 checksum. */
    uint32_t comp_size;      /* Compressed size. */
    uint32_t uncomp_size;    /* Uncompressed size. */
    uint16_t name_len;       /* Filename length. */
    uint16_t extra_len;      /* Extra data length. */
    uint16_t comment_len;    /* Comment length. */
    uint16_t disk_nbr_start; /* Disk nbr. where file begins. */
    uint16_t int_attrs;      /* Internal file attributes. */
    uint32_t ext_attrs;      /* External file attributes. */
    uint32_t lfh_offset;     /* Local File Header offset. */
} __attribute__((__packed__)) cfh;

#endif