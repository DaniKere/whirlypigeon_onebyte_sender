extern "C" {
  #include "user_interface.h"
}

uint8_t SENDER_ID = 1;   // Bacon_01=1, Bacon_02=2, Bacon_03=3
uint8_t VALUE     = 0;

uint8_t probe_frame[] = {
  // ---- 802.11 Probe Request ----
  0x40, 0x00,             // Frame Control
  0x00, 0x00,             // Duration
  0xff,0xff,0xff,0xff,0xff,0xff, // DA
  0x18,0xfe,0x34,0x12,0x34,0x56, // SA (tetszőleges)
  0xff,0xff,0xff,0xff,0xff,0xff, // BSSID
  0x00,0x00,             // Seq

  // ---- SSID tag (üres) ----
  0x00, 0x00,

  // ---- Vendor Specific IE ----
  0xdd, 0x05,             // ID, length
  0xAA, 0xBB, 0xCC,       // OUI
  0x00,                  // sender_id (helykitöltő)
  0x0F                   // value (helykitöltő)
};

void send_value(uint8_t v) {
  probe_frame[sizeof(probe_frame)-2] = SENDER_ID;
  probe_frame[sizeof(probe_frame)-1] = v;
  //Serial.print(probe_frame[0]);
  Serial.println(probe_frame[sizeof(probe_frame)-1]);
  wifi_send_pkt_freedom(probe_frame, sizeof(probe_frame), false);
}





//////////////////////////////////////////////////////////////////////////////////
/*RSSI MONITOR*/


const char* targetSSIDs[] = {
  "HUAWEI P20 Pro"
};
long long lastmillis = 0;
const int targetCount = 1;
int lastRSSI[targetCount] = {0};

// ---- RX Control ----
struct RxControl {
  signed rssi:8;
  unsigned rate:4;
  unsigned is_group:1;
  unsigned sig_mode:2;
  unsigned legacy_length:12;
  unsigned damatch0:1;
  unsigned damatch1:1;
  unsigned bssidmatch0:1;
  unsigned bssidmatch1:1;
  unsigned MCS:7;
  unsigned CWB:1;
  unsigned HT_length:16;
  unsigned Smoothing:1;
  unsigned Not_Sounding:1;
  unsigned Reserved:1;
  unsigned Aggregation:1;
  unsigned STBC:2;
  unsigned FEC_CODING:1;
  unsigned SGI:1;
  unsigned rxend_state:8;
  unsigned ampdu_cnt:8;
  unsigned channel:4;
  unsigned Reserved2:12;
};

// ---- Sniffer struktúra ----
struct sniffer_buf2 {
  struct RxControl rx_ctrl;
  uint8_t buf[112];
  uint16_t cnt;
  uint16_t len; 
};

// --- SSID kinyerés ---
bool extractSSID(const uint8_t *data, int len, String &outSSID) {
  int pos = 36; // beacon frame fixed offset
  while (pos + 2 < len) {
    uint8_t tag = data[pos];
    uint8_t tag_len = data[pos + 1];
    pos += 2;

    if (tag == 0) { // SSID tag
      outSSID = "";
      for (int i = 0; i < tag_len; i++) {
        if (pos + i >= len) break;
        outSSID += (char)data[pos + i];
      }
      return true;
    }
    pos += tag_len;
  }
  return false;
}

// ---- Sniffer callback ----
void ICACHE_FLASH_ATTR sniffer_cb(uint8_t *buf, uint16_t len) {
  if (len < sizeof(sniffer_buf2)) return;

  sniffer_buf2 *snif = (sniffer_buf2*) buf;
  const uint8_t *data = snif->buf;
  
  int data_len = snif->len;

  if (data_len < 36) return;
  if ((data[0] & 0xF0) != 0x80)  return; // csak beacon frame

  
  
  String ssid;
  if (!extractSSID(data, data_len, ssid)) return;

  for (int i = 0; i < targetCount; i++) {
    if (ssid == targetSSIDs[i]) {
       lastRSSI[i] = snif->rx_ctrl.rssi;      
    }
  }
}









/////////////////////////////////////////////////////////////////////////////////////




void setup() {
  Serial.begin(115200);
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(1);    // fix csatorna
   wifi_promiscuous_enable(1);
  //wifi_set_channel(1); // pl. csatorna 1, lehet váltani
  wifi_set_promiscuous_rx_cb(sniffer_cb);
  wifi_promiscuous_enable(1);
}

void loop() {
  VALUE = (uint8_t)(-1*lastRSSI[0]);
  send_value(VALUE);
  delay(127);              // ~50 packet/sec
}

