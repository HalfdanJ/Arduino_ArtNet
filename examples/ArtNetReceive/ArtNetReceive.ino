#include <SPI.h>
#include <Ethernet.h>
#include <ArtNet.h>

// Art-Net class instance with 1024 bytes buffer
ArtNet artnet = ArtNet(1024);

// Defaults
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 1);
IPAddress subnetmask(255, 255, 255, 0);
boolean useDhcp = false;
char *nodeName = "ArtNet Node";

void setup() {
  
  // Farstest SPI for ethernet
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  Ethernet.begin(mac, ip);
  
  // Init artnet
  artnet.setShortName(nodeName);
  artnet.setLongName(nodeName);
  artnet.setNumPorts(2);
  artnet.setPortAddress(0, 0); // Port 0, address 0
  artnet.setPortAddress(1, 1); // Port 1, address 1
  artnet.setNet(0);
  artnet.setSubNet(0);
  artnet.setPortType(0, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.setPortType(1, ARTNET_TYPE_OUTPUT | ARTNET_TYPE_DMX);
  artnet.begin(mac);
}

void loop() {

  int packetSize = artnet.parsePacket();
  if(packetSize) {

    switch(artnet.getOpCode()) {
      
      case ARTNET_OPCODE_POLL: {
        artnet.readPoll();
        artnet.sendPollReply();
      } break;
        
      case ARTNET_OPCODE_DMX: {
        artnet_dmx_header* header = artnet.readDmxHeader();
        byte port = artnet.getPortOutFromUni(header->subUni);
        switch(port) {
          case 0: break;
          case 1: break;
          default: artnet.flush();
        }
      } break;
      
      case ARTNET_OPCODE_IPPROG: {
        artnet_ip_prog* ipprog = artnet.readIpProg();
        
        if(ipprog->command == 0) {
          uint32_t ip = Ethernet.localIP();
          uint32_t sm = Ethernet.subnetMask();
          memcpy(ipprog->ip, &ip, 4);
          memcpy(ipprog->subnet, &sm, 4);
          ipprog->port = 0x1936;
          ipprog->status = useDhcp ? ARTNET_DHCP_ENABLED : 0;
          artnet.sendIpProgReply(ipprog);
        }
        else
        if(ipprog->command & ARTNET_IPCMD_IP) {
          artnet.stop();
          Ethernet.begin(mac, ipprog->ip);
          artnet.begin(mac);
          uint32_t i = Ethernet.localIP();
          uint32_t s = Ethernet.subnetMask();
          memcpy(ipprog->ip, &i, 4);
          memcpy(ipprog->subnet, &s, 4);          
          artnet.sendIpProgReply(ipprog);
        }
      } break;
      
      case ARTNET_OPCODE_ADDRESS: {
        artnet_address* address = artnet.readAddress();
        artnet.setShortName(address->shortName);
        artnet.setLongName(address->longName);
        artnet.setPortAddress(0, address->swOut[0]);
        artnet.setPortAddress(1, address->swOut[1]);
      } break;
      
      default:
        artnet.flush();
    }
  }
}
