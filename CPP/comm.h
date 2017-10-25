#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"

#ifndef COMM_H
#define COMM_H

class BaseComm {
    public:
        void transmit();
};

class XbeeComm:BaseComm {
    public:
        XbeeComm(const char* port);
        void handleXbeeFrame(const void FAR *frame, uint16_t length);
        xbee_dev_t *getXbee();
    private:
        xbee_serial_t serial;
        xbee_dev_t xbee;
};
#endif
