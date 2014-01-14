#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>

// Art-Net class instance with 1024 bytes buffer
ArtNet artnet = ArtNet(1024);

// Defaults
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

void setup() {
  
  // Farstest SPI for ethernet
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  Ethernet.begin(mac);
  
  // Init artnet
  artnet.setNumPorts(1);
  artnet.setPortAddress(0, 0); // Port 0, address 0
  artnet.setPortType(0, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.begin(mac);
}

void loop() {

  int packetSize = artnet.parsePacket();
  if(packetSize) {

    if(artnet.getOpCode() == ARTNET_OPCODE_DMX) {
        
      artnet_dmx_header* header = artnet.readDmxHeader();
      byte port = artnet.getPortOutFromUni(header->subUni);
      byte *dmx = artnet.readDmxData(header->length);

      // Do your DMX handling here!!
    }
    else {
      artnet.flush();
    }
  }
}
