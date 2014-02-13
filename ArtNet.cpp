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
    
    this->config = new ArtNetConfig();
    memset(this->config, 0, sizeof(ArtNetConfig));
    
    strcpy(config->shortName, "Art-Net Node");
    strcpy(config->longName, config->shortName);
    config->udpPort = ARTNET_UDP_PORT;
    config->dhcp = true;
    config->numPorts = 1;
    for (int i=0; i<4; i++) {
        config->portTypes[i] = ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX;
        config->portAddrIn[i] = i;
        config->portAddrOut[i] = i;
    }
}

ArtNet::ArtNet(ArtNetConfig & config, uint16_t bufferSize) {
    
    this->bufferSize = bufferSize;
    buffer = (uint8_t*)malloc(bufferSize);

    this->config = &config;
}

void ArtNet::begin() {
	udp.begin(config->udpPort);
}

void ArtNet::begin(const uint8_t* mac) {
    memcpy(config->mac, mac, 6);
	udp.begin(config->udpPort);
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
	this->config->net = net;
}

void ArtNet::setSubNet(uint8_t subnet) {
	this->config->subnet = subnet;
}

void ArtNet::setShortName(const char *shortName) {
	memcpy(this->config->shortName, shortName, 18);
}

char* ArtNet::getShortName() {
	return config->shortName;
}

void ArtNet::setLongName(const char *longName) {
	memcpy(this->config->longName, longName, 64);
}

char* ArtNet::getLongName() {
	return config->longName;
}

void ArtNet::setNumPorts(uint8_t numPorts) {
	this->config->numPorts = numPorts < 4 ? numPorts : 4;
}

void ArtNet::setPortType(uint8_t port, uint8_t type) {
	port &= 3;
	this->config->portTypes[port] = type;
}

void ArtNet::setPortAddress(uint8_t port, uint8_t address) {
	
	port &= 3;

	if (config->portTypes[port] & ARTNET_TYPE_INPUT)
		this->config->portAddrIn[port] = address;
	else if (config->portTypes[port] & ARTNET_TYPE_OUTPUT)
		this->config->portAddrOut[port] = address;
    else {
        this->config->portAddrIn[port] = address;
        this->config->portAddrOut[port] = address;
    }

}

void ArtNet::setMac(const uint8_t *mac) {
	memcpy(this->config->mac, mac, 6);
}

uint8_t ArtNet::getPortType(uint8_t port) {
	port &= 3;
	return this->config->portTypes[port];
}

uint8_t ArtNet::getPortAddress(uint8_t port) {
	port &= 3;
	return this->config->portAddrOut[port];
}

uint8_t ArtNet::getPortOutFromUni(uint8_t uni) {
    for (int i=0; i<4; i++) {
        if ((config->portAddrOut[i] == uni) ) {
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
    
    reply->port = config->udpPort;
    
    reply->verHi = config->verHi;
    reply->verLo = config->verLo;
    
    reply->netSwitch = config->net;
    reply->subSwitch = config->subnet;
    strcpy(reply->shortName, config->shortName);
    strcpy(reply->longName, config->longName);
    
    reply->numPortsLo = config->numPorts;
    memcpy(reply->portTypes, config->portTypes, 4);
    memcpy(reply->swIn, config->portAddrIn, 4);
    memcpy(reply->swOut, config->portAddrOut, 4);
    memcpy(reply->mac, config->mac, 6);
    
    
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
        memcpy(config->ip, ipprog->ip, 4);
        config->dhcp = false;
        stop();
        Ethernet.begin(config->mac, config->ip);
        begin();
    }
    else if(ipprog->command & ARTNET_IPCMD_DHCP) {
        config->dhcp = true;
        stop();
        Ethernet.begin(config->mac);
        uint32_t ip = Ethernet.localIP();
        memcpy(config->ip, &ip, 4);
        begin();
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
