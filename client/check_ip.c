#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "check_ip.h"

int check_number(int a, char *address){
    int i, correct;
    for(i = 0; i < a; i++){
        if(address[i] >= 48 && address[i] <= 57){
            correct = 0;
        } else{
            correct = -1;
            return correct;
        }
    }
    
	if(atoi(address) <= 255 && atoi(address) >= 0){
  	    correct = 0;
	}else correct = -1;
    return correct;
}

int check_ip(char *address){

    int slen_ip;        // Lenght of the received IP-address
    char address1[4], address2[4], address3[4], address4[4];    // Parts IP-address
    int correct = -1;    // Correct IP
    int point1, point2, point3, point4;  // Points IP-address
    int i;   // Counter

    slen_ip = strlen(address);

    //Checking lenght IP-address
    if(slen_ip > 15){
        printf("Error lenght IP-address\n");
        return correct;
    } else if(((strstr(address, ".") - address + 1) == 1)){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    } 

    point1 = strstr(address, ".") - address;
    if(point1 == 0){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    }
    strncpy(address1, address, point1);
    correct = check_number(point1, address1);
    if(correct != 0){
        printf("IP: incorrect\n");
        return correct;
    }
    address += point1 + 1;

    point2 = strstr(address, ".") - address;
    if(point2 == 0){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    }
    strncpy(address2, address, point2);
    correct = check_number(point2, address2);
    if(correct != 0){
        printf("IP: incorrect\n");
        return correct;
    }

    address += point2 + 1;

    point3 = strstr(address, ".") - address;
    if(point3 == 0){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    }
    strncpy(address3, address, point3);
    correct = check_number(point3, address3);
    if(correct != 0){
        printf("IP: incorrect\n");
        return correct;
    }
    address += point3 + 1;
    
    point4 = strchr(address, '\0') - address;
    if(point4 == 0){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    }
    strncpy(address4, address, point4);
    correct = check_number(point4-1, address4);
    if(correct != 0){
        printf("IP: incorrect\n");
        return correct;
    }

    if(strstr(address4, ".") != NULL){
        printf("IP: incorrect\n");
        printf("Error point\n");
        return correct;
    }

    printf("IP: correct\n");
    return correct;
}