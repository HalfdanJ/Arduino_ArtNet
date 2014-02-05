#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>

// ArtNet class instance with 1024 bytes buffer
ArtNet artnet = ArtNet(1024);

// Defaults
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

void setup() {
  
  // Farstest SPI for ethernet
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  // Initialize Ethernet shield
  Ethernet.begin(mac);
  
  // Initialize ArtNet handler
  artnet.setNumPorts(1); // Inputs or outputs. Maximum is 4.
  artnet.setPortAddress(0, 0); // Port 0, address 0
  artnet.begin(mac); // Start ArtNet handling
}

void loop() {

  // Receive ArtNet package from Ethernet shield
  if(artnet.parsePacket()) {
    
    // Read and handle packet
    artnet.handleAny();

    // Check packet type
    if(artnet.getOpCode() == ARTNET_OPCODE_DMX) {

      // Get header and dmx data
      byte* dmx = artnet.getDmxData();
      byte port = artnet.getDmxPort();

      // Do your DMX handling here!!
    }
  }
}
