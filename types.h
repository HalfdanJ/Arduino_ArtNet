#ifndef types_h
#define types_h

#include <inttypes.h>

typedef struct _artnet_header {
	uint8_t  id[8];
    uint16_t opCode;
} artnet_header;

typedef struct _artnet_poll {
	uint8_t protVerHi;
	uint8_t protVerLo;
	uint8_t talkToMe;
	uint8_t priority;
} artnet_poll;

typedef struct _artnet_poll_reply {
    uint8_t  ip[4];
    uint16_t port;
    uint8_t  verHi;
    uint8_t  verLo;
    uint8_t  netSwitch;
    uint8_t  subSwitch;
    uint8_t  oemHi;
    uint8_t  oemLo;
    uint8_t  ubeaVer;
    uint8_t  status;
    uint8_t  estaManLo;
    uint8_t  estaManHi;
    char     shortName[18];
    char     longName[64];
    char     nodeReport[64];
    uint8_t  numPortsHi;
    uint8_t  numPortsLo;
    uint8_t  portTypes[4];
    uint8_t  goodInput[4];
    uint8_t  goodOutput[4];
    uint8_t  swIn[4];
    uint8_t  swOut[4];
    uint8_t  swVideo;
    uint8_t  swMacro;
    uint8_t  swRemote;
    uint8_t  spare[3];
    uint8_t  style;
    uint8_t  mac[6];
    uint8_t  bindIp[4];
    uint8_t  bindIndex;
    uint8_t  status2;
    uint8_t  filler[26];
} artnet_poll_reply;

typedef struct _artnet_dmx_header {
	uint8_t  protVerHi;
	uint8_t  protVerLo;
	uint8_t  sequence;
	uint8_t  physical;
	uint8_t  subUni;
	uint8_t  net;
	uint16_t length;
} artnet_dmx_header;

typedef struct _artnet_ip_prog {
	uint8_t  protVerHi;
	uint8_t  protVerLo;
    uint8_t  filler1;
    uint8_t  filler2;
    uint8_t  command;
    uint8_t  filler4;
    uint8_t  ip[4];
    uint8_t  subnet[4];
    uint16_t port;
    uint8_t  status;
    uint8_t  spare[7];
} artnet_ip_prog;

typedef struct _artnet_address {
	uint8_t protVerHi;
	uint8_t protVerLo;
    uint8_t netSwitch;
    uint8_t filler2;
    char shortName[18];
    char longName[64];
    uint8_t swIn[4];
    uint8_t swOut[4];
    uint8_t subSwitch;
    uint8_t swVideo;
    uint8_t command;
} artnet_address;

typedef struct _artnet_tod_request {
    uint8_t protVerHi;
    uint8_t protVerLo;
    uint8_t filler[2];
    uint8_t spare[7];
    uint8_t net;
    uint8_t command;
    uint8_t addCount;
} artnet_tod_request;

typedef struct _artnet_tod_data {
    uint8_t  protVerHi;
    uint8_t  protVerLo;
    uint8_t  filler[2];
    uint8_t  spare[7];
    uint8_t  net;
    uint8_t  command;
    uint8_t  address;
    uint16_t uidTotal;
    uint8_t  blockCount;
    uint8_t  uidCount;
} artnet_tod_data;

#endif