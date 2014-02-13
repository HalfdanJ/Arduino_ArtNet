#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>

// ArtNet class instance
ArtNet artnet = ArtNet();

// Defaults
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192,168,1,1);

void setup() {

  // Initialize Ethernet shield
  Ethernet.begin(mac, ip);
  
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
