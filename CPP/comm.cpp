#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <thread>
#include <chrono>
#include <time.h>
#include <inttypes.h>

#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"

XbeeComm::XbeeComm(const char * port){
    memset(&(this->serial), 0, sizeof(xbee_serial_t));
    strncpy(this->serial.device, port, strlen(port));
    this->serial.baudrate = 115200;
    if (xbee_dev_init( &(this->xbee), &serial, NULL, NULL)){
        printf( "Failed to initialize XBee device.\n");
    }
    xbee_cmd_init_device( &(this->xbee));
    int status;
    do {
        xbee_dev_tick( &(this->xbee));
        status = xbee_cmd_query_status( &(this->xbee));
    } while (status == -EBUSY);
}

void XbeeComm::handleXbeeFrame(const void FAR *frame, uint16_t length){
    printf("HANDLING FRAME\n");
    for(int i = 0; i < length; i++){
        printf("%d\t", i);
    }
    printf("\n");
    char *b = (char*) frame;
    for(int i = 0; i < length; i++){
        printf("%x\t", (*(b+i)&0xFF));
    }
    printf("\n");
}
xbee_dev_t *XbeeComm::getXbee(){
    return &(this->xbee);
}

void XbeeComm::handleNIFrame( const void *frame, uint16_t length){;
    char *buf = (char*) frame;
    uint64_t address = getAddress(buf+4);
    char *name = (char*) calloc(getNiStringLength(buf+22), sizeof(char));
    memcpy(name, buf+22, getNiStringLength(buf+22));
    printf("\nNI FRAME FROM: 0x");
    printf("%" PRIx64 " NAME: %s\n", address, name);
    printf("REGISTERED\n");
    XbeeDev *xbd = new XbeeDev(name, getNiStringLength(buf+22), address);
    this->Devices[address] = xbd;
}

uint64_t getAddress(const char *data){
    uint64_t address = 0;
    address |= (*(data)&0xFFULL)<<(8*7);
    address |= (*(data+1)&0xFFULL)<<(8*6);
    address |= (*(data+2)&0xFFULL)<<(8*5);
    address |= (*(data+3)&0xFFULL)<<(8*4);
    address |= (*(data+4)&0xFFULL)<<(8*3);
    address |= (*(data+5)&0xFFULL)<<(8*2);
    address |= (*(data+6)&0xFFULL)<<(8);
    address |= (*(data+7)&0xFFULL);
    return address;
}

uint16_t getNiStringLength(const char *data){
    int i = 0;
    while(1){
        if(*(data+i) == '\0') break;
        i++;
    }
    return i+1;
}


XbeeDev::XbeeDev(const char* name, uint16_t length, uint64_t addr){
    this->name = (char*) calloc(length+1, sizeof(char));
    memcpy(this->name, name, length);
    this->addr64bit = addr;
}

XbeeDev::~XbeeDev(){
    free(this->name);
}
