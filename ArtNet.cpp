/*
 *  ArtNet.cpp
 *  
 *  Created by Tobias Ebsen on 6/2/13.
 *
 */

#include "ArtNet.h"

ArtNet::ArtNet(uint16_t bufferSize) : ArtNetCore(bufferSize) {
}

ArtNet::ArtNet(ArtNetConfig & config, uint16_t bufferSize) : ArtNetCore(config, bufferSize) {
}

void ArtNet::begin() {
    begin(config->mac);
}

void ArtNet::begin(uint8_t* mac) {
    
    memcpy(config->mac, mac, 6);

    if (config->dhcp && Ethernet.begin(mac) != 0) {
        uint32_t i = Ethernet.localIP();
        memcpy(config->ip, &i, 4);
        uint32_t m = Ethernet.subnetMask();
        memcpy(config->mask, &m, 4);
    }
    else
        Ethernet.begin(mac, config->ip);
    
    udp.begin(config->udpPort);

    ArtNet::createPollReply();
    sendBuffer(sizeof(artnet_header)+sizeof(artnet_poll_reply), true);
}

void ArtNet::begin(uint8_t* mac, IPAddress ip) {

    memcpy(config->mac, mac, 6);
    
    uint32_t i = ip;
    memcpy(config->ip, &i, 4);
    
    config->dhcp = false;
    
    Ethernet.begin(mac, ip);
    
    udp.begin(config->udpPort);

    ArtNet::createPollReply();
    sendBuffer(sizeof(artnet_header)+sizeof(artnet_poll_reply), true);
}

int ArtNet::parsePacket() {

    if (udp.parsePacket() > 0) {

        int r = udp.read(ArtNet::buffer, min(udp.available(), ArtNet::bufferSize));
        if (r > sizeof(artnet_header) && ArtNet::isValidPacket())
            return r;
    }
    return 0;
}

void ArtNet::maintain() {

    if (Ethernet.maintain() != DHCP_CHECK_NONE) {

        if (!(Ethernet.localIP() == config->ip)) {
            uint32_t i = Ethernet.localIP();
            memcpy(config->ip, &i, 4);
            uint32_t m = Ethernet.subnetMask();
            memcpy(config->mask, &m, 4);

            ArtNet::createPollReply();
            sendBuffer(sizeof(artnet_header)+sizeof(artnet_poll_reply), true);
        }
    }
}

void ArtNet::handlePoll() {
    ArtNet::createPollReply();
    sendBuffer(sizeof(artnet_header)+sizeof(artnet_poll_reply), false);
}

void ArtNet::handleIpProg() {
    ArtNet::handleIpProg();
    
    if (ArtNet::getIpCommand() & ARTNET_IPCMD_PROGRAM) {

        udp.stop();
        
        if (config->dhcp && Ethernet.begin(config->mac) != 0) {
            uint32_t i = Ethernet.localIP();
            memcpy(config->ip, &i, 4);
            uint32_t m = Ethernet.subnetMask();
            memcpy(config->mask, &m, 4);
        }
        else
            Ethernet.begin(config->mac, config->ip);
        
        udp.begin(config->udpPort);
    }
    
    ArtNet::createIpProgReply();
    sendBuffer(sizeof(artnet_header)+sizeof(artnet_ip_prog), false);
}

void ArtNet::handleAny() {

    switch (ArtNet::getOpCode()) {

        case ARTNET_OPCODE_POLL:
            handlePoll();
            break;

        case ARTNET_OPCODE_IPPROG:
            handleIpProg();
            break;

        case ARTNET_OPCODE_ADDRESS:
            ArtNet::handleAddress();
            break;
    }
}

void ArtNet::sendBuffer(int size, bool broadcast) {
    if (broadcast)
        udp.beginPacket(IPAddress(255,255,255,255), udp.remotePort());
    else
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write(ArtNet::buffer, size);
    udp.endPacket();
}
