#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include <map>
#include <thread>


#ifndef COMM_H
#define COMM_H

#define TIMEOUT 10

#ifdef __cplusplus
extern "C" {
#endif

class XbeeDev {
    public:
        XbeeDev(const char* name, uint16_t length, uint64_t addr);
        ~XbeeDev();
        uint64_t addr64bit;
        char * name;
        time_t last_check;
};

class BaseComm {
    public:
        void transmit();
};

class XbeeComm:BaseComm {
    public:
        XbeeComm(const char* port);
        void handleXbeeFrame(const void FAR *frame, uint16_t length);
        void handleNIFrame(const void FAR *frame, uint16_t length);
        xbee_dev_t *getXbee();
        std::map<uint64_t, XbeeDev*> Devices;
    private:
        void commLoop();
        xbee_serial_t serial;
        xbee_dev_t xbee;
        std::thread online_check;
};

uint64_t getAddress(const char *data);
uint16_t getNiStringLength(const char *data);

void onlineCheck(XbeeDev *xbeec);

#ifdef __cplusplus
}
#endif

#endif
