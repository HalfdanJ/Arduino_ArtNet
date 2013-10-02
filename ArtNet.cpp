/*
 *  ArtNet.cpp
 *  
 *  Created by Tobias Ebsen on 6/2/13.
 *
 */

#include "ArtNet.h"

#define ARTNET_UDP_PORT		0x1936

static uint8_t artnetID[] = {'A', 'r', 't', '-', 'N', 'e', 't', 0x00};



ArtNet::ArtNet() {
	net = 0;
	subnet = 0;
	shortName = NULL;
	longName = NULL;
	numPorts = 0;
	for (int i=0; i<4; i++) {
		portTypes[i] = 0;
		portAddrIn[i] = i;
		portAddrOut[i] = i;
	}
	mac = NULL;
}

void ArtNet::begin() {
	begin(NULL);
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
	
	// Read header ID
	uint8_t id[8];
	udp.read(id, sizeof(id));
	
	// Check ID
	if (memcmp(id, artnetID, sizeof(artnetID)) != 0) {
		udp.flush();
		return 0;
	}
	
	//Read OpCode
	udp.read((uint8_t*)&opCode, sizeof(opCode));
	
	return n;
}

uint16_t ArtNet::getOpCode() {
	return opCode;
}

void ArtNet::setNet(uint8_t net) {
	this->net = net;
}

void ArtNet::setSubNet(uint8_t subnet) {
	this->subnet = subnet;
}

void ArtNet::setShortName(const char *shortName) {
	this->shortName = shortName;
}

void ArtNet::setLongName(const char *longName) {
	this->longName = longName;
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

	if (portTypes[port] & ARTNET_TYPE_OUTPUT)
		this->portAddrOut[port] = address;
}

void ArtNet::setMac(const uint8_t *mac) {
	this->mac = mac;
}

void ArtNet::readPoll(artnet_poll *poll) {
	udp.read((uint8_t*)poll, sizeof(poll));
}

void ArtNet::readPoll() {
	artnet_poll poll;
	readPoll(&poll);
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

void ArtNet::readDmxData(uint8_t *data, uint16_t length) {
	
	udp.read(data, length);
}

void ArtNet::flush() {
	udp.flush();
}

void ArtNet::sendPollReply() {

	udp.beginPacket(udp.remoteIP(), udp.remotePort());

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


	udp.endPacket();
}