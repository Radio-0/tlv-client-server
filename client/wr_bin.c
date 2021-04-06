#include <stdlib.h>
#include "tlv_info.h"
#include "crc32.h"

static uint8_t data_default[] = {	
	0x21, 0x05, 'B', 'R', '1', '0', '0',
	0x22, 0x0d, 'B', 'R', '1', '0', '0', '-', '1', '2', 'F', '8', 'X', 'C', 'I',
	0x23, 0x08, '0', '1', '1', '3', '6', '1', '2', '6',
	0x24, 0x06, 0xd8, 0xe0, 0xb8, 0x00, 0x8d, 0x1e,
	0x25, 0x04, 'a', 'u', 't', 'o',
    0x26, 0x01, '4',
	0x27, 0x01, '1',
	0x28, 0x1d, 'x', '8', '6', '_', '6', '4', '-', 'b', 'u', 'l', 'a', 't', '_', 'b', 'r', '1', '0', '0', '_', '1', '2', 'f', '8', 'x', 'c', 'i', '-', 'r', '3',
	0x29, 0x07, '2', '0', '2', '0', '.', '0', '2',
	0x2A, 0x02, '2', '2',
	0x2B, 0x08, 'B', 'U', 'L', 'A', 'T', ' ', 'L', 'L', 'C',
	0x2C, 0x01, '7',
	0x2D, 0x08, 'B', 'U', 'L', 'A', 'T', ' ', 'L', 'L', 'C',
	0x2E, 0x7, 'B', 'M', 'C', '_', '1', '.', '0', '.', '0',
    0x2F, 
	0xFE, 0x4, 0xfa, 0x3d, 0xde, 0x63,
};

struct tlvinfo_tag_key array_tag_key[] = {
	0x21, "ProductName",
	0x22, "PartNumber",
	0x23, "SerialNumber",
	0x24, "BaseMAC",
	0x25, "ManufactureDate",
	0x26, "DeviceVersion",
	0x27, "LabelRevision",
	0x28, "PlatformName",
	0x29, "OnieVersion",
	0x2A, "NumberOfMACs",
	0x2B, "Manufacturer",
	0x2C, "CountryCode",
	0x2D, "Vendor",
	0x2E, "DiagVersion",
	0x2F, "ServiceTag",
	0xFD, "VendorExtension",
};

void tlv_fill_str(tlvinfo_tlv_t *tlv, uint8_t type, char *val){    
	size_t len = strlen(val);
	tlv->tag = type;
	tlv->length = len;
	memmove(tlv->value_tlv, val, len);
	tlv->value_tlv[len] = 0;
}

void tlv_fill_mac(tlvinfo_tlv_t *tlv, uint8_t type, char *val){
    int j;
	uint8_t mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    sscanf(val, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    tlv->tag = type;
    tlv->length = MAX_MAC_LEN;
    for (j = 0; j < MAX_MAC_LEN; ++j)   {
        memcpy(&tlv->value_tlv[j], &mac[j], 1);
        tlv->value_tlv[tlv->length] = 0;
    }
}

void tlv_fill_crc(tlvinfo_tlv_t *tlv, unsigned long val){    
	size_t len = MAX_CRC_LEN;
	tlv->tag = TLV_CODE_CRC_32;
	tlv->length = len;
	memmove(tlv->value_tlv, &val, len);
	tlv->value_tlv[len] = 0;
}

tlvinfo_header_t *get_tlv_data(FILE *file)
{

	char str[100];			// Received buffer from eeprom.conf
	char key[30];			// Received name key
	char value[100];		// Received value
	unsigned long crc32;	// Checksum value
	int number_array;		// Number of array elemets
    int i;					// Counter

    static uint8_t data[TLV_INFO_MAX_LEN];
    static tlvinfo_header_t *header = (tlvinfo_header_t *)data;
    static uint8_t tlv_buf[TLV_ENTRY_MAX_LEN];
	static tlvinfo_tlv_t* tlv = (tlvinfo_tlv_t *)tlv_buf;

	FILE *eprom = fopen("/home/semin/sc/client-server/client/eeprom.conf", "r");
	if(!file){
		printf("File .bin is not opened\n\n");
		exit(1);
	}

	if(!eprom){
		printf("File eeprom.conf is not opened\n\n");
		exit(1);
	}

	number_array = sizeof(array_tag_key) / sizeof(array_tag_key[0]);	
	// Getting a string from eeprom.conf
	while(fgets(str, sizeof(str), eprom)){
		if(strstr(str, "=") == NULL)
			continue;
		if(strstr(str, "#"))
			continue;
		sscanf(str, "%[^'=']=%[^'\n']", key, value);
        int value_len = strlen(value);				
		for(i = 0; i < number_array; i++){
			if(key && !strcmp(array_tag_key[i].key_tlv, key)){
				// Writing MAC
				if(!strcmp(array_tag_key[i].key_tlv, "MAC")){
                    tlv_fill_mac(tlv, array_tag_key[i].tag_tlv, value);
                    memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
                    header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;
				}
				// Writing all data not MAC
				else {
                    tlv_fill_str(tlv, array_tag_key[i].tag_tlv, value);                                                                     
                    memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
                    header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;
				}
			}
		}
	}
	// Calculation and writing Checksum
    crc32 = onlp_crc32(0, header->data, header->datalen);
    tlv_fill_crc(tlv, crc32);
    memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
    header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;

	// Writing all data in file
    fwrite(header->data, sizeof(uint8_t), header->datalen, file);

	fclose(eprom);
    printf("\n");
    return header;
}