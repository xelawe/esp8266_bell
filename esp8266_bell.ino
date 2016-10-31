#define serdebug
#ifdef serdebug
#define DebugPrint(...) {  Serial.print(__VA_ARGS__); }
#define DebugPrintln(...) {  Serial.println(__VA_ARGS__); }
#else
#define DebugPrint(...) { }
#define DebugPrintln(...) { }
#endif


#include "tools_wifiman.h"
#include "ota_tool.h"

#include <WiFiUdp.h>

#define BellSoundPin 0
#define BellButtonPin 2
#define BellNbr 1

unsigned int localPort = 44444;      // local port to listen for packets
unsigned int BellPort = 44444;      // port to send Bell packets
IPAddress BroadcastIP(255, 255, 255, 255); // time.nist.gov NTP server
const int Bell_PACKET_SIZE = 8;

byte packetBuffer[ Bell_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

void setup() {
#ifdef serdebug
  Serial.begin(115200);
#endif

  // initialize the sound-Module pin as an output:
  pinMode(BellSoundPin, OUTPUT);
  digitalWrite(BellSoundPin, HIGH);

  // initialize the pushbutton pin as an input:
  pinMode(BellButtonPin, INPUT);

  wifi_init("ESPBell");

  init_ota("ESPBell");

  DebugPrintln("Starting UDP");
  udp.begin(localPort);
  DebugPrint("Local port: ");
  DebugPrintln(udp.localPort());

}

void loop() {

  check_ota();

  if (digitalRead(BellButtonPin) == LOW) {
    sendBellpacket(BroadcastIP); // send an NTP packet to a time server

    delay(1000);
  }
  rcvBellpacket();
  delay(250);

}

void playBell(int Bellnbr) {

  byte test;
  test = B10000000;

  // One Wire Communication with WT588D Board
  digitalWrite(BellSoundPin, LOW);
  delay(5);
  //  delay(500);


  for (int i = 0; i < 8; i++) {
    digitalWrite(BellSoundPin, HIGH);
    if (bitRead(test, i) == 1) {
      delayMicroseconds(600);
      digitalWrite(BellSoundPin, LOW);
      delayMicroseconds(300);
    }
    else {
      delayMicroseconds(300);
      digitalWrite(BellSoundPin, LOW);
      delayMicroseconds(600);
    }
  }
  digitalWrite(BellSoundPin, HIGH);


}

void rcvBellpacket() {
  int cb = udp.parsePacket();
  if (!cb) {
    return;
  }

  DebugPrint("packet received, length=");
  DebugPrintln(cb);
  // We've received a packet, read the data from it
  udp.read(packetBuffer, Bell_PACKET_SIZE); // read the packet into the buffer

  DebugPrint("Bell#: ");
  DebugPrintln(packetBuffer[0]);
  playBell(1);

}

// send an Bell Signal to all Receivers on the net
unsigned long sendBellpacket(IPAddress& address)
{
  DebugPrintln("sending Bell packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, Bell_PACKET_SIZE);
  packetBuffer[0] = BellNbr;   //Bell Number


  // all Bell fields have been given values, now
  // you can send a packet:
  udp.beginPacket(address, BellPort);
  udp.write(packetBuffer, Bell_PACKET_SIZE);
  udp.endPacket();
}
