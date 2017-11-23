#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdexcept>
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
    this->check = time(NULL);
}

bool XbeeComm::sendCommand(XbeeDev *dev, char order, char *data, uint8_t datalen){
    Command *cmd = new Command(dev->addr64bit, xbee_next_frame_id(&this->xbee), order, this->getNextId(), data, datalen);
    this->Commands.insert(this->Commands.end(), cmd);
    cmd->send(&this->xbee);
    return true;
}

uint8_t XbeeComm::getNextId(){
    this->frameid++;
    return this->frameid;
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

void XbeeComm::onlineLoop(){
    if(time(NULL) - this->check > 1){
        this->check = time(NULL);
        for(std::map<uint64_t, XbeeDev*>::iterator it = this->Devices.begin(); it != this->Devices.end(); ++it){
            printf("\n0x%" PRIx64 " %s\n", it->first, it->second->name);
            if(this->check - it->second->last_check < TIMEOUT && this->check - it->second->last_check > PING){
                printf("SENDING PING");
                this->sendPing(it->second);
            } else if(this->check - it->second->last_check > TIMEOUT){
                printf("\nRemoving 0x%" PRIx64 " %s\n", it->first, it->second->name);
                it->second->~XbeeDev();
                this->Devices.erase(it);
            }
        }
    }
}

void XbeeComm::sendPing(XbeeDev *dev){
    Command *cmd = new Command(dev->addr64bit, xbee_next_frame_id(&this->xbee), (char)2, this->getNextId(), NULL, 0) ;
    cmd->send(&this->xbee);
}

void XbeeComm::handleRXFrame(const void *frame, uint16_t length){
    char *buf = (char*) frame;
    uint64_t address = getAddress(buf+1);
    try {
        XbeeDev *d = this->Devices.at(address);
        d->last_check = time(NULL);
        char *data = (char*) calloc(length - 12, sizeof(char));
        char order = *data+1;
        char iden = *data+2;
        char len = *data+3;
        std::vector<Command*>::iterator it;
        for(it = this->Commands.begin(); it != this->Commands.end(); it++){
            if((*it)->address == address && (*it)->order == order && (*it)->iden == iden){
                (*it)->setReady();
                memcpy((*it)->data, data, (uint8_t)len);
                this->Commands.erase(it);
            } 
        }
        memcpy(data, buf+12, length-12);
        printf("\nReceived: %s", data);
    } catch (std::out_of_range){
        printf("\nMessage from Unregistered device: %" PRIx64 "\n", address);
    }
}

void XbeeComm::handleNIFrame( const void *frame, uint16_t length){
    char *buf = (char*) frame;
    uint64_t address = getAddress(buf+1);
    char *name = (char*) calloc(getNiStringLength(buf+22), sizeof(char));
    memcpy(name, buf+22, getNiStringLength(buf+22));
    printf("\nNI FRAME FROM: 0x");
    printf("%" PRIx64 " NAME: %s\n", address, name);
    printf("REGISTERED\n");
    XbeeDev *xbd = new XbeeDev(name, getNiStringLength(buf+22), address);
    xbd->last_check = time(NULL);
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

Command::Command(uint64_t address, uint8_t id, char order, char iden, char *payload, uint8_t payload_length){
    this->address = address;
    this->header[1] = id;
    this->header[2] = (address>>(8*7))&0xFF;
    this->header[3] = (address>>(8*6))&0xFF;
    this->header[4] = (address>>(8*5))&0xFF;
    this->header[5] = (address>>(8*4))&0xFF;
    this->header[6] = (address>>(8*3))&0xFF;
    this->header[7] = (address>>(8*2))&0xFF;
    this->header[8] = (address>>(8*1))&0xFF;
    this->header[9] = (address>>(8*0))&0xFF;
    this->header[15] = order;
    this->header[16] = iden;
    this->header[17] = payload_length;
    memcpy(this->data, payload, payload_length);
    this->datalen = payload_length;
}

void Command::send(xbee_dev_t *xb){
    xbee_frame_write(xb, this->header, this->headerlen, this->data, this->datalen, XBEE_WRITE_FLAG_NONE);
}

void Command::setReady(){
    this->_ready = true;
}
