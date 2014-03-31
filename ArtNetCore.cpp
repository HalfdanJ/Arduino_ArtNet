/*
 *  ArtNetCore.cpp
 *  
 *  Created by Tobias Ebsen on 6/2/13.
 *
 */

#include <stdio.h>
#include <stdlib.h> // malloc
#include <string.h> // memcpy
#include "ArtNetCore.h"

static uint8_t artnetID[] = {'A', 'r', 't', '-', 'N', 'e', 't', 0x00};

ArtNetCore::ArtNetCore(uint16_t bufferSize) {

    this->bufferSize = bufferSize;
    buffer = (uint8_t*)malloc(bufferSize);
    
    config = new ArtNetConfig();
    setDefaultConfig();
}

ArtNetCore::ArtNetCore(ArtNetConfig & config, uint16_t bufferSize) {
    
    this->bufferSize = bufferSize;
    buffer = (uint8_t*)malloc(bufferSize);

    this->config = &config;
}

ArtNetConfig & ArtNetCore::getConfig() {
    return *config;
}

void ArtNetCore::setDefaultConfig() {

    config->mac[0] = 0xDE;
    config->mac[1] = 0xAD;
    config->mac[2] = 0xBE;
    config->mac[3] = 0xEF;
    config->mac[4] = 0xFE;
    config->mac[5] = 0xED;
    
    config->ip[0] = 192;
    config->ip[1] = 168;
    config->ip[2] = 1;
    config->ip[3] = 1;
    
    config->mask[0] = 255;
    config->mask[1] = 255;
    config->mask[2] = 255;
    config->mask[3] = 0;
    
    config->udpPort = ARTNET_UDP_PORT;
    
    config->dhcp = true;
    
    strcpy(config->shortName, "Art-Net Node");
    strcpy(config->longName, config->shortName);
    
    config->numPorts = 1;
    for (int i=0; i<4; i++) {
        config->portTypes[i] = ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX;
        config->portAddrIn[i] = i;
        config->portAddrOut[i] = i;
    }
}

uint8_t* ArtNetCore::getBuffer() {
    return buffer;
}

int ArtNetCore::getSize() {
    return bufferSize;
}

bool ArtNetCore::isValidPacket() {
    return (memcmp(buffer, artnetID, sizeof(artnetID)) == 0);
}

uint16_t ArtNetCore::getOpCode() {
	return buffer[8] | (buffer[9] << 8);
}

void ArtNetCore::setOpCode(uint16_t opCode) {
	buffer[8] = opCode & 0xFF;
    buffer[9] = opCode >> 8;
}

uint8_t* ArtNetCore::getDmxData() {
    return (buffer+sizeof(artnet_header)+sizeof(artnet_dmx_header));
}

uint16_t ArtNetCore::getDmxLength() {
    return ((artnet_dmx_header*)(buffer+sizeof(artnet_header)))->length;
}

uint8_t ArtNetCore::getDmxPort() {
    uint8_t uni = ((artnet_dmx_header*)(buffer+sizeof(artnet_header)))->subUni;
    return getPortOutFromUni(uni);
}

uint8_t ArtNetCore::getIpCommand() {
    return ((artnet_ip_prog*)(buffer+sizeof(artnet_header)))->command;
}

uint8_t ArtNetCore::getPortOutFromUni(uint8_t uni) {
    for (int i=0; i<4; i++) {
        if ((config->portAddrOut[i] == uni) ) {
            return i;
        }
    }
    return -1;
}

void ArtNetCore::createPollReply() {

    memcpy(buffer, artnetID, sizeof(artnetID));
    setOpCode(ARTNET_OPCODE_POLLREPLY);
    memset(buffer+sizeof(artnet_header), 0, sizeof(artnet_poll_reply));
    
    artnet_poll_reply* reply = (artnet_poll_reply*)(buffer+sizeof(artnet_header));
    
    memcpy(reply->ip, config->ip, 4);
    
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
    reply->status2 = 0x0C | (config->dhcp << 1);
}

void ArtNetCore::createIpProgReply() {
    
    memcpy(buffer, artnetID, sizeof(artnetID));
    setOpCode(ARTNET_OPCODE_IPREPLY);
    
    artnet_ip_prog *ipprog = (artnet_ip_prog*)(buffer+sizeof(artnet_header));
    
    ipprog->protVerLo = 14;
    
    memcpy(ipprog->ip, config->ip, 4);
    memcpy(ipprog->subnet, config->mask, 4);
    ipprog->port = ((config->udpPort & 0xff) << 8) | (config->udpPort >> 8);
    ipprog->status = config->dhcp ? 0 : ARTNET_DHCP_ENABLED;
}

void ArtNetCore::handleIpProg() {
    artnet_ip_prog* ipprog = (artnet_ip_prog*)(buffer+sizeof(artnet_header));
    
    if (ipprog->command & ARTNET_IPCMD_PORT)
        config->udpPort = ((ipprog->port & 0xff) << 8) | (ipprog->port >> 8);
    if (ipprog->command & ARTNET_IPCMD_IP)
        memcpy(config->ip, ipprog->ip, 4);
    if (ipprog->command & ARTNET_IPCMD_SUBNET)
        memcpy(config->mask, ipprog->subnet, 4);
    config->dhcp = (ipprog->command & ARTNET_IPCMD_DHCP) != 0;
}

void ArtNetCore::handleAddress() {
    artnet_address* address = (artnet_address*)(buffer+sizeof(artnet_header));
    
    memcpy(config->shortName, address->shortName, 18);
    memcpy(config->longName, address->longName, 64);
    
    for(int i=0; i<4; i++) {
        config->portAddrIn[i] = address->swIn[i];
        config->portAddrOut[i] = address->swOut[i];
    }
}

