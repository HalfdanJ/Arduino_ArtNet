/*
 *  ArtNet.h
 *
 *  Created by Tobias Ebsen
 *
 *  Implementation of the Art-Net protocol for use with the Arduino platform.
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of GNU General Public License version 3 as
 *  published by the Free Software Foundation.
 *
 *  Art-Net(TM) Designed by and Copyright Artistic Licence Holdings Ltd
 *
 */

#ifndef ArtNet_h
#define ArtNet_h

#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include "ArtNetCore.h"

class ArtNet : public ArtNetCore {
public:

    // Constructors
    ArtNet(uint16_t bufferSize = 530);
    ArtNet(ArtNetConfig & config, uint16_t bufferSize = 530);

    void begin();
    void begin(uint8_t* mac);
    void begin(uint8_t* mac, IPAddress ip);
    int  parsePacket();
    
    void maintain();
    
    void handlePoll();
    void handleIpProg();
    void handleAny();

private:
    void sendBuffer(int size, bool broadcast = false);

    EthernetUDP udp;
};

#endif