#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <thread>
#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include "comm.h"

XbeeComm *xbee;
xbee_serial_t serial;
xbee_dev_t my_xbee;

int echo_handler(xbee_dev_t *xbee, const void FAR *frame, uint16_t length, void FAR *context){
    XbeeComm *comm = (XbeeComm*) context;
    /*char* fra = (char*) frame; 
    printf("Received %d bytes, Frame Type %x\n", length, ((uint8_t)*fra) & 0xFF);
    for(uint8_t i = 1; i < length; i++){
        printf("%d: ", i);
        printf("%x\n", *(char *)(fra + i) & 0xFF);
        printf("\n");
    }*/
    comm->handleXbeeFrame(frame, length);
    return 0;
}

#define XBEE_FRAME_HANDLE_ECHO { 0, 0, echo_handler, NULL }
#define PORT "/dev/ttyUSB0"

const xbee_dispatch_table_entry_t xbee_frame_handlers[] = {
    { 90, 0, echo_handler, xbee },
    XBEE_FRAME_HANDLE_LOCAL_AT,
    XBEE_FRAME_TABLE_END
};

void loop(){
    while(1){
        xbee_dev_tick(xbee->getXbee());
        //xbee_cmd_tick();
        fflush(stdout);
    }
}

int scan_cb(const xbee_cmd_response_t FAR * response){
    uint_fast8_t len = response->value_length;
    printf("Received %d bytes\n", len);
    for(int i = 0; i < len; i++){
        printf("%d\t", i);
    }
    printf("\n");
    char *b = (char*) response->value_bytes;
    for(int i = 0; i < len; i++){
        printf("%x\t", (*(b+i)&0xFF));
    }
    printf("\n");
    return XBEE_ATCMD_REUSE;
}

int main(void) {
    xbee = new XbeeComm("/dev/ttyUSB0");
    std::thread l (loop);
    int16_t command = xbee_cmd_create(xbee->getXbee(), "ND");
    xbee_cmd_set_callback(command, scan_cb, NULL);
    sleep(5);
    int res = xbee_cmd_send(command);
    if(!res){
        printf("Command Sent\n");
    } else {
        printf("Command Error: %d\n", res);
    };
    l.join();
    return 0;
}
