Art-Net for Arduino
==============

A lightweight Art-Net library for the Arduino platform.
This fast and effective library supports up to 4 Art-Net universes. The practical number of universes depends on the transmission speed required. The standard Ethernet shield and library will only be able to receive 1 universe at 30fps. If you need more universes and/or faster framerates thare are a number of factors that can speed up the transmission.

## Speeding up ArtNet
Following things can be done to speed up ArtNet reception:

* SPI speed. Default is quarter clock frequency, but can be set to half clock frequency (8 MHz on a standard Arduino):
```cpp
SPI.setClockDivider(SPI_CLOCK_DIV2);
```
* Memory allocation. The Ethernet chip has 16 Kb of memory, which is divided between 8 sockets by default. Since ArtNet is only using one UDP socket we can re-allocate all the memory to that socket. In order to do so, it is necessary to use a different version of the standard Ethernet library: [WIZ_Ethernet_Library](https://github.com/media-architecture/WIZ_Ethernet_Library)
Once installed, it allows you to set up the memory allocation like this:
```cpp
uint16_t sizes[8] = {(16<<10),0,0,0,0,0,0,0}; // 16 Kb memory to the first socket
W5100.setRXMemorySizes(sizes);
```
* Unicast instead of broadcast. If your software broadcasts Art-Net packets to more than one Art-Net node, they will all receive all of the packets and need to decide for them self wether to handle them or not. This is completely unnecessary and will slow down your Art-Net stream.

* A better Ethernet shield. The standard Ethernet shield uses the WIZnet W5100 controller chip, which is the slowest and has the least amount of memory of all the WIZnet chips. Upgrading to a shield with the W5200 chip will increase the speed of your hardware.

* 20 MHz Arduino? Since the SPI timing is a multiple of the Arduino clock rate, it is possible to squeeze even more speed out of the Ethernet shield by running the AVR microcontroller at 20 MHz. Please note that this may mess up timings in other Arduino functions, such as delay() and delayMicroseconds(), not to mention libraries for controlling LEDs.
