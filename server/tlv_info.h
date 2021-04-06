#ifndef __TLV_INFO_H__
#define __TLV_INFO_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>
#include <arpa/inet.h>

#define TLV_INFO_MAX_LEN        2048
#define TLV_TOTAL_LEN_MAX       (TLV_INFO_MAX_LEN - sizeof(tlvinfo_header_t))
#define TLV_ENTRY_MAX_LEN       256
#define TLV_VALUE_MAX_LEN       (TLV_ENTRY_MAX_LEN - sizeof(tlvinfo_tlv_t))
#define MAX_KEY_LEN				64 
#define MAX_MAC_LEN             6
#define MAX_CRC_LEN             4
#define TLV_CODE_CRC_32         0xFE


typedef struct __attribute__ ((__packed__)) tlvinfo_header_s {
    uint16_t    datalen;
    uint8_t     data[0];
} tlvinfo_header_t;

typedef struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    uint8_t  tag;
    uint8_t  length;
    uint8_t  value_tlv[0];
} tlvinfo_tlv_t;

struct tlvinfo_tag_key {
	uint8_t     tag_tlv; 
    char        key_tlv[MAX_KEY_LEN];      
};

#endif //_TLV_INFO_H_
