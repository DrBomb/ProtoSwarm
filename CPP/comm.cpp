#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"

XbeeComm::XbeeComm(const char * port){
    memset(&(this->serial), 0, sizeof(xbee_serial_t));
    strncpy(this->serial.device, port, strlen(port));
    this->serial.baudrate = 9600;
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
