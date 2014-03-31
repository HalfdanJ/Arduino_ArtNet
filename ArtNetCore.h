/*
 *  ArtNetCore.h
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

#ifndef ArtNetCore_h
#define ArtNetCore_h

#include <inttypes.h>

#include "constants.h"
#include "types.h"

typedef struct _ArtnetConfig {
    uint8_t  mac[6];
    uint8_t  ip[4];
    uint8_t  mask[4];
    uint16_t udpPort;
    uint8_t  dhcp;
    uint8_t  net;
    uint8_t  subnet;
    char     shortName[18];
    char     longName[64];
    uint8_t  numPorts;
    uint8_t  portTypes[4];
    uint8_t  portAddrIn[4];
    uint8_t  portAddrOut[4];
    uint8_t  verHi;
    uint8_t  verLo;
} ArtNetConfig;

class ArtNetCore {
public:

    // Constructors
    ArtNetCore(uint16_t bufferSize);
    ArtNetCore(ArtNetConfig & config, uint16_t bufferSize);
    
    // Config access
    ArtNetConfig & getConfig();
    void     setDefaultConfig();
    
    // Buffer access
    uint8_t* getBuffer();
    int      getSize();

    // Packet header check
    bool     isValidPacket();
	
    // Opcode accessors
	uint16_t getOpCode();
    void     setOpCode(uint16_t opCode);

    // DMX accessors
    uint8_t* getDmxData();
    uint16_t getDmxLength();
    uint8_t  getDmxPort();
    
    // IpProg accessors
    uint8_t  getIpCommand();
    
    // Internal accessors
    uint8_t  getPortOutFromUni(uint8_t uni);
	
    // Packet creators
    void createDmx();
    void createPoll();
	void createPollReply();
    void createIpProgReply();

    // Packet handlers
    virtual void handleIpProg();
    virtual void handleAddress();

protected:
    uint8_t* buffer;
    uint16_t bufferSize;
    
    ArtNetConfig *config;
};

#endif