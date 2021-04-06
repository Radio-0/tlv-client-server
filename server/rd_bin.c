/*Reading from a binary file and displaying it to the screen*/

#include "crc32.h"
#include "tlv_info.h"
#include "data_rd.h"

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
	0xFE,
};

void tlv_fill_str(tlvinfo_tlv_t *tlv, uint8_t type, char *val){    
	size_t len = strlen(val);
	tlv->tag = type;
	tlv->length = len;
	memmove(tlv->value_tlv, val, len);
	tlv->value_tlv[len] = 0;
}

void tlv_fill_mac(tlvinfo_tlv_t *tlv, uint8_t type, uint8_t val[MAX_MAC_LEN]){
    int i;
    tlv->tag = type;
    tlv->length = MAX_MAC_LEN;
    for(i = 0; i < MAX_MAC_LEN; i++){
        memmove(&tlv->value_tlv[i], &val[i], MAX_MAC_LEN);
	    tlv->value_tlv[tlv->length] = 0;
    }
}

void tlv_fill_crc(tlvinfo_tlv_t *tlv, unsigned long val){    
    int i;
	size_t len = MAX_CRC_LEN;
	tlv->tag = TLV_CODE_CRC_32;
	tlv->length = len;
	memmove(tlv->value_tlv, &val, len);
	tlv->value_tlv[len] = 0;
}

int get_str_data(FILE *fd){
    
    uint8_t slen;   // Lenght data
    uint8_t tag;    // Tag data
    char *value;            // Buffer values
    char name_key[100];     // Key names
    int file_len = 0;       // File size
    int number_array;       // Number of array elemets
    int count, i;           // Counters
    unsigned long crc32;    // Checksum value
    char *buf = (char *) malloc(sizeof(char) * 64); // Buffer for writing data
    if (buf == NULL){
        printf("Error memory\n");
        return -1;
    }

    static uint8_t data[TLV_INFO_MAX_LEN];
    static tlvinfo_header_t *header = (tlvinfo_header_t *)data;
    memset(header, 0, TLV_INFO_MAX_LEN * sizeof(uint8_t));

    static uint8_t tlv_buf[TLV_ENTRY_MAX_LEN];
	static tlvinfo_tlv_t* tlv = (tlvinfo_tlv_t *)tlv_buf;
    
    if(!fd){
        printf("\nFile .bin is not opened\n\n");
		return -1;
    }

    // File start position
    fseek(fd, 0, SEEK_END);
    file_len = ftell(fd);
    count = 0;
    number_array = sizeof(array_tag_key) / sizeof(array_tag_key[0]);

    while(count < (file_len - (MAX_CRC_LEN + 2))){
        tag = 0x0;
        slen = 0;
        value = (char *) malloc(sizeof(char) * 64);
        if (value == NULL){
            printf("Error\n");
            free(buf);
            return -1;
        }
        // Reading tag and slen of file
        fseek(fd, count, SEEK_SET);       
        fread(&tag, sizeof(uint8_t), 1,fd);
        count++;
        fseek(fd, count, SEEK_SET);
        fread(&slen, sizeof(uint8_t), 1, fd);
        count++;
        fseek(fd, count, SEEK_SET);

        // Reading value for MAC
        if(tag == 0x24 && slen == MAX_MAC_LEN){
            uint8_t mac[MAX_MAC_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
            char mac_val[4];
            for(i = 0; i < MAX_MAC_LEN; i++){
                fread(&mac[i], sizeof(uint8_t), 1, fd);
                if(i == 0) 
                    snprintf(mac_val, 8, "%x", mac[i]);
                else
                    snprintf(mac_val, 9, ":%x", mac[i]);
                strncat(value, mac_val, strlen(mac_val));
            }
            for(i = 0; i < number_array; i++){
                if(tag == array_tag_key[i].tag_tlv)
                    snprintf(name_key, strlen(array_tag_key[i].key_tlv), "%s", array_tag_key[i].key_tlv);
            }
            tlv_fill_mac(tlv,  tag, mac);
            memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
            header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;
            snprintf(buf, (strlen(name_key) + strlen(value) + 1), "%s=%s", name_key, value);
        }
        // Reading value for data not MAC
        else{
            fread(value, sizeof(char), slen, fd);
            value[slen] = '\0';
            for(i = 0; i < number_array; i++){
            if(tag == array_tag_key[i].tag_tlv)
                snprintf(name_key, strlen(array_tag_key[i].key_tlv), "%s", array_tag_key[i].key_tlv);
            }
            tlv_fill_str(tlv, tag, value);
            memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
            header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;
            snprintf(buf, (strlen(name_key) + strlen(value) + 1 + 1), "%s=%s", name_key, value);
        }
        count += slen;
        free(value);
        printf("%s\n", buf);
    }
    crc32 = onlp_crc32(0, header->data, header->datalen);
    tlv_fill_crc(tlv, crc32);   
    memcpy(&header->data[header->datalen], tlv,  sizeof(tlvinfo_tlv_t) + tlv->length);
    header->datalen += sizeof(tlvinfo_tlv_t) + tlv->length;
        
    fseek(fd, file_len - (MAX_CRC_LEN + 2), SEEK_END);
    uint8_t crc_val[MAX_CRC_LEN] = {0x0, 0x0, 0x0, 0x0};    // Received from file CRC
    uint8_t crc_check[MAX_CRC_LEN] = {0x0, 0x0, 0x0, 0x0};  // Data CRC

    if (crc_val == NULL) {
        printf("Error\n");
        free(buf);
        return -1;
    }

    fseek(fd, count, SEEK_SET);       
    fread(&tag, sizeof(uint8_t), 1,fd);
    count++;
    fseek(fd, count, SEEK_SET);
    fread(&slen, sizeof(uint8_t), 1, fd);
    count++;
    fseek(fd, count, SEEK_SET);

    // Checking CRC
    for(i = 0; i < MAX_CRC_LEN; i++){
        fread(&crc_val[i], sizeof(uint8_t), 1, fd);
        crc_check[i] = header->data[i+header->datalen - MAX_CRC_LEN];
        if(crc_check[i] != crc_val[i]){
            printf("Error crc32\n");
            free(buf);
            return -1;
        } 
    }
        
    free(buf); 
    return 0;
}