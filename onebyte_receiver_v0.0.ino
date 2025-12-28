long long lastmillis = 0;
const int targetCount = 3;
int lastRSSI[targetCount+1] = {0,99,99,99};

extern "C" {
  #include "user_interface.h"
}

/* ---- SAJÁT, MINIMÁL RxControl ---- */
struct RxControl {
  signed rssi : 8;
  unsigned rate : 4;
  unsigned is_group : 1;
  unsigned : 1;
  unsigned sig_mode : 2;
  unsigned legacy_length : 12;
  unsigned damatch0 : 1;
  unsigned damatch1 : 1;
  unsigned bssidmatch0 : 1;
  unsigned bssidmatch1 : 1;
  unsigned MCS : 7;
  unsigned CWB : 1;
  unsigned HT_length : 16;
  unsigned : 1;
  unsigned : 1;
  unsigned : 1;
  unsigned Aggregation : 1;
  unsigned STBC : 2;
  unsigned FEC_CODING : 1;
  unsigned SGI : 1;
  unsigned rxend_state : 8;
  unsigned ampdu_cnt : 8;
  unsigned channel : 4;
  unsigned : 12;
};

struct PromiscuousPkt {
  RxControl rx_ctrl;
  uint8_t payload[0];
};

void sniffer_cb(uint8_t *buf, uint16_t len) {
  PromiscuousPkt *pkt = (PromiscuousPkt*) buf;

  int sig_len = pkt->rx_ctrl.legacy_length;
  if (sig_len < 30) return;

  uint8_t *frame = pkt->payload;

  // Probe Request?
  if (frame[0] != 0x40) return;

  int i = 24;
  while (i + 2 < sig_len) {
    uint8_t tag  = frame[i];
    uint8_t tlen = frame[i+1];
    if (i + 2 + tlen > sig_len) break;

    if (tag == 0xDD && tlen >= 5 &&
        frame[i+2] == 0xAA &&
        frame[i+3] == 0xBB &&
        frame[i+4] == 0xCC) {

      lastRSSI[frame[i+5]] = frame[i+6];
      uint8_t sender = frame[i+5];
      uint8_t value  = frame[i+6];

      /*Serial.print("Bacon_");
      Serial.print(sender);
      Serial.print(" | value=");
      Serial.print(value);
      Serial.print(" | RSSI=");
      Serial.println(pkt->rx_ctrl.rssi);*/
    }
    i += tlen + 2;
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(1);

  wifi_promiscuous_enable(0);
  wifi_set_promiscuous_rx_cb(sniffer_cb);
  wifi_promiscuous_enable(1);

  Serial.println("RSSI_MONITOR READY");
}

void loop() {

   if (millis()-lastmillis > 300){

      //char msg[32];
      //sprintf(msg, "%d,%d,%d", lastRSSI[0], lastRSSI[1], lastRSSI[2]);
     //if(Serial.available())
     
      Serial.println(String(lastRSSI[1])+";"+String(lastRSSI[2])+";"+(String)lastRSSI[3]);
      //Serial.println(String(lastProbe[1])+";"+String(lastProbe[2])+";"+(String)lastProbe[3]);
     
     /*for(int i =0; i<3; i++){ 
        //Serial.print(lastRSSI[i]);
        Serial.print(";");
        //Serial.println(lastRSSI[i]);
     }
      Serial.println("");*/
     
     lastmillis=millis();    
   }
  }

