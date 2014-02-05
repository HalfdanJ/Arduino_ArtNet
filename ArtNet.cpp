/*
 *  ArtNet.cpp
 *  
 *  Created by Tobias Ebsen on 6/2/13.
 *
 */

#include "ArtNet.h"

#define ARTNET_UDP_PORT		0x1936

static uint8_t artnetID[] = {'A', 'r', 't', '-', 'N', 'e', 't', 0x00};



ArtNet::ArtNet(uint16_t bufferSize) {
    this->bufferSize = bufferSize;
    buffer = (uint8_t*)malloc(bufferSize);
	net = 0;
	subnet = 0;
	shortName[0] = 0;
	longName[0] = 0;
	numPorts = 0;
	for (int i=0; i<4; i++) {
		portTypes[i] = ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX;
		portAddrIn[i] = i;
		portAddrOut[i] = i;
	}
	mac = NULL;
}

void ArtNet::begin(const uint8_t *mac) {
	this->mac = mac;
	udp.begin(ARTNET_UDP_PORT);
}

void ArtNet::stop() {
	udp.stop();
}

int ArtNet::parsePacket() {
	
	int n = udp.parsePacket();
	
	// Check for minimum package size
	if (n < sizeof(artnet_header)) {
		udp.flush();
		return 0;
	}
	
	// Read header ID + OpCode
    udp.read(buffer, sizeof(artnet_header));
	
	// Check ID
	if (memcmp(buffer, artnetID, sizeof(artnetID)) != 0) {
		udp.flush();
		return 0;
	}
    
	return n;
}

uint16_t ArtNet::getOpCode() {
	return buffer[8] | (buffer[9] << 8);
}

void ArtNet::setOpCode(uint16_t opCode) {
	buffer[8] = opCode & 0xFF;
    buffer[9] = opCode >> 8;
}

void* ArtNet::getData() {
    return (buffer+sizeof(artnet_header));
}

uint8_t* ArtNet::getDmxData() {
    return (buffer+sizeof(artnet_header)+sizeof(artnet_dmx_header));
}

uint16_t ArtNet::getDmxLength() {
    return ((artnet_dmx_header*)(buffer+sizeof(artnet_header)))->length;
}

uint8_t ArtNet::getDmxPort() {
    uint8_t uni = ((artnet_dmx_header*)(buffer+sizeof(artnet_header)))->subUni;
    return getPortOutFromUni(uni);
}

uint8_t ArtNet::getIpCommand() {
    return ((artnet_ip_prog*)(buffer+sizeof(artnet_header)))->command;
}

void ArtNet::setNet(uint8_t net) {
	this->net = net;
}

void ArtNet::setSubNet(uint8_t subnet) {
	this->subnet = subnet;
}

void ArtNet::setShortName(const char *shortName) {
	memcpy(this->shortName, shortName, 18);
}

char* ArtNet::getShortName() {
	return shortName;
}

void ArtNet::setLongName(const char *longName) {
	memcpy(this->longName, longName, 64);
}

char* ArtNet::getLongName() {
	return longName;
}

void ArtNet::setNumPorts(uint8_t numPorts) {
	this->numPorts = numPorts < 4 ? numPorts : 4;
}

void ArtNet::setPortType(uint8_t port, uint8_t type) {
	port &= 3;
	this->portTypes[port] = type;
}

void ArtNet::setPortAddress(uint8_t port, uint8_t address) {
	
	port &= 3;

	if (portTypes[port] & ARTNET_TYPE_INPUT)
		this->portAddrIn[port] = address;
	else if (portTypes[port] & ARTNET_TYPE_OUTPUT)
		this->portAddrOut[port] = address;
    else {
        this->portAddrIn[port] = address;
        this->portAddrOut[port] = address;
    }

}

void ArtNet::setMac(const uint8_t *mac) {
	this->mac = mac;
}

uint8_t ArtNet::getPortType(uint8_t port) {
	port &= 3;
	return this->portTypes[port];
}

uint8_t ArtNet::getPortAddress(uint8_t port) {
	port &= 3;
	return this->portAddrOut[port];
}

uint8_t ArtNet::getPortOutFromUni(uint8_t uni) {
    for (int i=0; i<4; i++) {
        if ((portAddrOut[i] == uni) ) {
            return i;
        }
    }
    return -1;
}

void ArtNet::readPoll() {
	udp.read(buffer+sizeof(artnet_header), sizeof(artnet_poll));
}

void ArtNet::readDmx() {
    artnet_dmx_header* header = (artnet_dmx_header*)getData();
	udp.read((uint8_t*)header, sizeof(artnet_dmx_header));
	header->length = ((header->length & 0xff) << 8) | (header->length >> 8);
    uint8_t* data = buffer+sizeof(artnet_header)+sizeof(artnet_dmx_header);
	udp.read(data, header->length);
}

void ArtNet::readIpProg() {
    artnet_ip_prog *ipprog = (artnet_ip_prog*)getData();
    udp.read((uint8_t*)ipprog, sizeof(artnet_ip_prog));
}

void ArtNet::readAddress() {
    artnet_address *address = (artnet_address*)getData();
    udp.read((uint8_t*)address, sizeof(artnet_address));
}

void ArtNet::flush() {
	udp.flush();
}

void ArtNet::sendPollReply() {

    memcpy(buffer, artnetID, sizeof(artnetID));
    setOpCode(ARTNET_OPCODE_POLLREPLY);
    memset(buffer+sizeof(artnet_header), 0, sizeof(artnet_poll_reply));
    
    artnet_poll_reply* reply = (artnet_poll_reply*)getData();
    
    uint32_t ip = Ethernet.localIP();
    memcpy(reply->ip, &ip, 4);
    
    reply->port = ARTNET_UDP_PORT;
    
    reply->netSwitch = net;
    reply->subSwitch = subnet;
    strcpy(reply->shortName, shortName);
    strcpy(reply->longName, longName);
    
    reply->numPortsLo = numPorts;
    memcpy(reply->portTypes, portTypes, 4);
    memcpy(reply->swIn, portAddrIn, 4);
    memcpy(reply->swOut, portAddrOut, 4);
    memcpy(reply->mac, mac, 6);
    
    
    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(buffer, sizeof(artnet_header)+sizeof(artnet_poll_reply));
    udp.endPacket();
}

void ArtNet::sendIpProgReply() {
    
    memcpy(buffer, artnetID, sizeof(artnetID));
    setOpCode(ARTNET_OPCODE_IPREPLY);
    
    artnet_ip_prog *ipprog = (artnet_ip_prog*)getData();
    
    uint32_t i = Ethernet.localIP();
    uint32_t s = Ethernet.subnetMask();
    memcpy(ipprog->ip, &i, 4);
    memcpy(ipprog->subnet, &s, 4);
    ipprog->port = ARTNET_UDP_PORT;
    ipprog->status = Ethernet.maintain() == DHCP_CHECK_NONE ? 0 : ARTNET_DHCP_ENABLED;

	udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(buffer, sizeof(artnet_header)+sizeof(artnet_ip_prog));
    udp.endPacket();
}

void ArtNet::handlePoll() {
    readPoll();
    sendPollReply();
}

void ArtNet::handleDmx() {
    readDmx();
}

void ArtNet::handleIpProg() {
    readIpProg();
    artnet_ip_prog* ipprog = (artnet_ip_prog*)getData();
    
    if(ipprog->command & ARTNET_IPCMD_IP) {
        stop();
        Ethernet.begin((uint8_t*)mac, ipprog->ip);
        begin(mac);
    }
    else if(ipprog->command & ARTNET_IPCMD_DHCP) {
        stop();
        Ethernet.begin((uint8_t*)mac);
        begin(mac);
    }
    sendIpProgReply();
}

void ArtNet::handleAddress() {
    readAddress();
    artnet_address* address = (artnet_address*)getData();
    
    setShortName(address->shortName);
    setLongName(address->longName);
    
    for(int i=0; i<4; i++) {
        setPortAddress(i, address->swOut[i]);
    }
    sendPollReply();    
}

void ArtNet::handleAny() {

    uint16_t opcode = getOpCode();
    switch (opcode) {
            
        case ARTNET_OPCODE_POLL:
            handlePoll(); break;

        case ARTNET_OPCODE_DMX:
            handleDmx(); break;

        case ARTNET_OPCODE_IPPROG:
            handleIpProg(); break;
            
        case ARTNET_OPCODE_ADDRESS:
            handleAddress(); break;

        default:
            udp.flush();
    }
}
