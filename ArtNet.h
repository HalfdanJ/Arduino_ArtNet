/*
 *  ArtNet.h
 *
 *  Created by Tobias Ebsen on 6/2/13.
 *
 */

#ifndef ArtNet_h
#define ArtNet_h

#include <Ethernet.h>
#include <EthernetUdp.h>

#define ARTNET_OPCODE_POLL		0x2000
#define ARTNET_OPCODE_POLLREPLY	0x2100
#define ARTNET_OPCODE_DMX		0x5000

#define ARTNET_TYPE_DMX		0
#define ARTNET_TYPE_MIDI	1
#define ARTNET_TYPE_AVAB	2
#define ARTNET_TYPE_CMX		3
#define ARTNET_TYPE_ADB		4
#define ARTNET_TYPE_ARTNET	5

#define ARTNET_TYPE_INPUT	0x40
#define ARTNET_TYPE_OUTPUT	0x80

typedef struct _artnet_poll {
	uint8_t protVerHi;
	uint8_t protVerLo;
	uint8_t talkToMe;
	uint8_t priority;
} artnet_poll;

typedef struct _artnet_dmx_header {
	uint8_t protVerHi;
	uint8_t protVerLo;
	uint8_t sequence;
	uint8_t physical;
	uint8_t subUni;
	uint8_t net;
	uint16_t length;
} artnet_dmx_header;


class ArtNet {
public:

	ArtNet();
	
	void begin();
	void begin(const uint8_t *mac);
	void stop();
	
	void setNet(uint8_t net);
	void setSubNet(uint8_t subnet);
	void setShortName(const char *shortName);
	void setLongName(const char *longName);
	void setNumPorts(uint8_t numPorts);
	void setPortType(uint8_t port, uint8_t type);
	void setPortAddress(uint8_t port, uint8_t address);
	void setMac(const uint8_t *mac);
	
	int parsePacket();
	
	uint16_t getOpCode();
	
	void readPoll(artnet_poll *poll);
	void readPoll();
	void readDmx(artnet_dmx_header *header, uint8_t *data, uint16_t length);
	void readDmxHeader(artnet_dmx_header *header);
	void readDmxData(uint8_t *data, uint16_t length);
	
	void flush();
	
	void sendPollReply();

private:
	EthernetUDP udp;
	uint16_t opCode;
	uint8_t net;
	uint8_t subnet;
	const char *shortName;
	const char *longName;
	uint8_t numPorts;
	uint8_t portTypes[4];
	uint8_t portAddrIn[4];
	uint8_t portAddrOut[4];
	const uint8_t *mac;
};

#endif