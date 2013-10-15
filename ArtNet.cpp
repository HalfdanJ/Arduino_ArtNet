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
		portTypes[i] = 0;
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
	if (n < 10) {
		udp.flush();
		return 0;
	}
	
	// Read header ID + OpCode
    udp.read(buffer, 10);
	
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

uint8_t ArtNet::getPortOutFromUni(uint8_t uni) {
    for (int i=0; i<4; i++) {
        if ((portAddrOut[i] == uni) ) {
            return i;
        }
    }
    return -1;
}


void ArtNet::readPoll(artnet_poll *poll) {
	udp.read((uint8_t*)poll, sizeof(poll));
}

artnet_poll* ArtNet::readPoll() {
	udp.read(buffer+10, sizeof(artnet_poll));
}

void ArtNet::readDmx(artnet_dmx_header *header, uint8_t *data, uint16_t length) {
	
	udp.read((uint8_t*)header, sizeof(artnet_dmx_header));
	header->length = ((header->length & 0xff) << 8) | (header->length >> 8);
	
	if(length > header->length)
		length = header->length;
	
	udp.read(data, length);
}

void ArtNet::readDmxHeader(artnet_dmx_header *header) {
	
	udp.read((uint8_t*)header, sizeof(artnet_dmx_header));
	header->length = ((header->length & 0xff) << 8) | (header->length >> 8);
}

artnet_dmx_header* ArtNet::readDmxHeader() {
	
    artnet_dmx_header* header = (artnet_dmx_header*)(buffer+10);
	udp.read((uint8_t*)header, sizeof(artnet_dmx_header));
	header->length = ((header->length & 0xff) << 8) | (header->length >> 8);
    return (artnet_dmx_header*)header;
}

void ArtNet::readDmxData(uint8_t *data, uint16_t length) {
	
	udp.read(data, length);
}

uint8_t* ArtNet::readDmxData(uint16_t length) {
	
    uint8_t* data = buffer+10+sizeof(artnet_dmx_header);
	udp.read(data, length);
    return data;
}

void ArtNet::readIpProg(artnet_ip_prog *ipprog) {
    udp.read((uint8_t*)ipprog, sizeof(artnet_ip_prog));
}

artnet_ip_prog* ArtNet::readIpProg() {
    artnet_ip_prog *ipprog = (artnet_ip_prog*)(buffer+10);
    udp.read((uint8_t*)ipprog, sizeof(artnet_ip_prog));
    return ipprog;
}

void ArtNet::readAddress(artnet_address *address) {
    udp.read((uint8_t*)address, sizeof(artnet_address));
}

artnet_address* ArtNet::readAddress() {
    artnet_address *address = (artnet_address*)(buffer+10);
    udp.read((uint8_t*)address, sizeof(artnet_address));
    return address;
}

void ArtNet::flush() {
	udp.flush();
}

void ArtNet::sendPollReply() {

    memset(buffer, 0, 10+sizeof(artnet_poll_reply));
    memcpy(buffer, artnetID, sizeof(artnetID));
    buffer[8] = ARTNET_OPCODE_POLLREPLY & 0xFF;
    buffer[9] = ARTNET_OPCODE_POLLREPLY >> 8;
    
    artnet_poll_reply* reply = (artnet_poll_reply*)(buffer+10);
    
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
    udp.write(buffer, 10+sizeof(artnet_poll_reply));
    udp.endPacket();

	/*udp.beginPacket(udp.remoteIP(), udp.remotePort());

	udp.write(artnetID, sizeof(artnetID));
	
	udp.write((uint8_t)(ARTNET_OPCODE_POLLREPLY & 0xFF));
	udp.write((uint8_t)(ARTNET_OPCODE_POLLREPLY >> 8));
	
	IPAddress ip = Ethernet.localIP();
	udp.write(ip[0]);
	udp.write(ip[1]);
	udp.write(ip[2]);
	udp.write(ip[3]);
	
	udp.write((uint8_t)(ARTNET_UDP_PORT & 0xFF));
	udp.write((uint8_t)(ARTNET_UDP_PORT >> 8));
	
	udp.write((uint8_t)0); // VersInfoH
	udp.write((uint8_t)0); // VersInfo

	udp.write(net);
	udp.write(subnet);
	
	udp.write((uint8_t)0); // OemHi
	udp.write((uint8_t)0); // Oem
	udp.write((uint8_t)0); // Ubea Version
	udp.write((uint8_t)0); // Status1
	udp.write((uint8_t)0); // EstaManLo
	udp.write((uint8_t)0); // EstaManHi
	
	int n = shortName == NULL ? 0 : strlen(shortName);
	if (n > 17) n = 17;
	udp.write((uint8_t*)shortName, n);
	n = 18 - n;
	for (int i=0; i<n; i++)
		udp.write((uint8_t)0);
	
	n = longName == NULL ? 0 : strlen(longName);
	if (n > 63) n = 63;
	udp.write((uint8_t*)longName, n);
	n = 64 - n;
	for (int i=0; i<n; i++)
		udp.write((uint8_t)0);

	for (int i=0; i<64; i++)
		udp.write((uint8_t)0); // NodeReport
	
	udp.write((uint8_t)0); // NumPortsHi
	udp.write(numPorts);   // NumPortsLo
	
	udp.write(portTypes, sizeof(portTypes));

	for (int i=0; i<8; i++)
		udp.write((uint8_t)0); // GoodInput + GoodOutput
	
	udp.write(portAddrIn, sizeof(portAddrIn));
	udp.write(portAddrOut, sizeof(portAddrOut));

	for (int i=0; i<7; i++)
		udp.write((uint8_t)0); //

	if (mac != NULL) {
		udp.write(mac, 6);
	} else {
		for (int i=0; i<6; i++)
			udp.write((uint8_t)0);
	}


	udp.endPacket();*/
}

void ArtNet::sendIpProgReply(artnet_ip_prog *ipprog) {
    
	udp.beginPacket(udp.remoteIP(), udp.remotePort());

	udp.write(artnetID, sizeof(artnetID));
	
	udp.write((uint8_t)(ARTNET_OPCODE_IPREPLY & 0xFF));
	udp.write((uint8_t)(ARTNET_OPCODE_IPREPLY >> 8));
    
    udp.write((uint8_t*)ipprog, sizeof(artnet_ip_prog));
    
    udp.endPacket();
}