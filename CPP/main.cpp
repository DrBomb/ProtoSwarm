#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"

class XBEE_Backbone{
    public:
        void test();
};

void XBEE_Backbone::test(){
    printf("Hello\n");
}

xbee_serial_t serial;
xbee_dev_t my_xbee;

int echo_handler(xbee_dev_t *xbee, const void FAR *frame, uint16_t length, void FAR *context){
    char* fra = (char*) frame; 
    printf("Received %d bytes, Frame Type %x\n", length, ((uint8_t)*fra) & 0xFF);
    for(uint8_t i = 1; i < length; i++){
        printf("%d: ", i);
        printf("%x\n", *(char *)(fra + i) & 0xFF);
        printf("\n");
    }
    return 0;
}

#define XBEE_FRAME_HANDLE_ECHO { 0, 0, echo_handler, NULL }
#define PORT "/dev/ttyUSB0"

const xbee_dispatch_table_entry_t xbee_frame_handlers[] = {
    { 0, 0, echo_handler, NULL },
    XBEE_FRAME_TABLE_END
};

int main(void) {
    XBEE_Backbone test = XBEE_Backbone();
    test.test();
    memset(&serial, 0, sizeof(xbee_serial_t));
    strncpy(serial.device,PORT,sizeof(PORT)-1);
    serial.baudrate = 9600;
    if (xbee_dev_init( &my_xbee, &serial, NULL, NULL)){
        printf( "Failed to initialize XBee device.\n");
        return -1;
    }
    xbee_cmd_init_device( &my_xbee);
    while(1){
        xbee_dev_tick(&my_xbee);
    }
    printf("%s",serial.device);
    return 0;
}
