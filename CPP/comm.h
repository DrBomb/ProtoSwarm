#include "xbee/platform.h"
#include "xbee/device.h"
#include "xbee/atcmd.h"
#include <map>
#include <vector>
#include <thread>


#ifndef COMM_H
#define COMM_H

#define TIMEOUT 10
#define PING 5

#ifdef __cplusplus
extern "C" {
#endif

class Command {
    public:
        bool ready();
        void setReady();
        char *response();
        Command(uint64_t address, uint8_t id, char order, char iden, char* payload, uint8_t payload_length);
        void send(xbee_dev_t* xb);
        char order;
        uint64_t address;
        char iden;
        char data[20];
    private:
        char header[18] = {(char)0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (char)0xFF, (char)0xFE, 0x00, 0x00, '(', 0x00, 0x00};
        uint16_t headerlen=18, datalen;
        bool _ready;
        char *res_buf = NULL;
};

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
        bool sendCommand(XbeeDev *dev, char order, char *data, uint8_t datalen);
        void handleXbeeFrame(const void FAR *frame, uint16_t length);
        void handleNIFrame(const void FAR *frame, uint16_t length);
        void handleRXFrame(const void FAR *frame, uint16_t length);
        uint8_t getNextId();
        xbee_dev_t *getXbee();
        std::map<uint64_t, XbeeDev*> Devices;
        void onlineLoop();
    private:
        void commLoop();
        xbee_serial_t serial;
        xbee_dev_t xbee;
        std::thread online_check;
        time_t check;
        void sendPing(XbeeDev *dev);
        std::vector<Command*> Commands;
        uint8_t frameid=0;
};

uint64_t getAddress(const char *data);
uint16_t getNiStringLength(const char *data);

void onlineCheck(XbeeDev *xbeec);

#ifdef __cplusplus
}
#endif

#endif
