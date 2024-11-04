#include <SPI.h>
#include <mcp2515.h>
#include <PS4Controller.h>
#include "esp_gap_bt_api.h"

struct can_frame canMsg;
MCP2515 CAN(5);

uint32_t currentTick,nowTick,currentTick1,nowTick1;
int mode=1;

//remove device bluetooth DS4
void removePairedDevices() {
  uint8_t pairedDeviceBtAddr[20][6];
  int count = esp_bt_gap_get_bond_device_num();
  esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
  for (int i = 0; i < count; i++) {
    esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
  }
}

void setup() {
  // while(!Serial);
  Serial.begin(115200);
  SPI.begin();
  CAN.reset();
  CAN.setBitrate(CAN_250KBPS, MCP_8MHZ);
  CAN.setNormalMode();
  PS4.begin("e8:61:7e:0a:54:34"); //84:30:95:52:99:66 94:08:53:39:c5:c8 e8:61:7e:0a:54:34 (rexus 8C:41:F2:70:F8:4C)
  removePairedDevices(); // This helps to solve connection issues
  Serial.println("Ready.");
}

void loop() {
  currentTick = millis();
  currentTick1 = millis();
  canMsg.can_id = 0x36; //CAN id as 0x036
  canMsg.can_dlc = 8; //CAN data length as 8
  if (PS4.isConnected()) {
    // Membuat variabel byte untuk menggabungkan data kondisi
    uint8_t data[8] = {0};
    int x = PS4.LStickX();
    int y = PS4.LStickY();
    int lx = abs(x);
    int ly = abs(y);

    // Menggunakan bit masking untuk menggabungkan kondisi tombol
    if (PS4.Right()) 	  canMsg.data[0] |= (1 << 0);
    if (PS4.Down()) 	  canMsg.data[0] |= (1 << 1);
    if (PS4.Up()) 	    canMsg.data[0] |= (1 << 2);
    if (PS4.Left()) 	  canMsg.data[0] |= (1 << 3);
    if (PS4.Square()) 	canMsg.data[0] |= (1 << 4);
    if (PS4.Cross()) 	  canMsg.data[0] |= (1 << 5);
    if (PS4.Circle()) 	canMsg.data[0] |= (1 << 6);
    if (PS4.Triangle()) canMsg.data[0] |= (1 << 7);

    if (PS4.L1()) 	    canMsg.data[1] |= (1 << 0);
    if (PS4.R1()) 	    canMsg.data[1] |= (1 << 1);
    if (PS4.L3()) 	    canMsg.data[1] |= (1 << 2);
    if (PS4.R3()) 	    canMsg.data[1] |= (1 << 3);
    if (PS4.Share()) 	  canMsg.data[1] |= (1 << 4);
    if (PS4.Options()) 	canMsg.data[1] |= (1 << 5);
    if (PS4.PSButton()) canMsg.data[1] |= (1 << 6);
    if (PS4.Touchpad()) canMsg.data[1] |= (1 << 7);

    if (PS4.L2()) 	    canMsg.data[2] = PS4.L2Value();
    if (PS4.R2()) 	    canMsg.data[3] = PS4.R2Value();
    if (PS4.LStickX()>0){
      canMsg.data[4] = lx;
      canMsg.data[5] = 0;
    }
    else{
      canMsg.data[4] = 0;
      canMsg.data[5] = lx;
    }

    if (PS4.LStickY()>0){
      canMsg.data[7] = ly;
      canMsg.data[6] = 0;
    }
    else{
      canMsg.data[7] = 0;
      canMsg.data[6] = ly;
    }
    
    CAN.sendMessage(&canMsg); //Sends the CAN message
    // Serial.print("c0: ");   Serial.print(canMsg.data[0]);
    // Serial.print("  c1: "); Serial.print(canMsg.data[1]);
    // Serial.print("  c2: "); Serial.print(canMsg.data[2]);
    // Serial.print("  c3: "); Serial.print(canMsg.data[3]);
    // Serial.print("  lx: "); Serial.print(canMsg.data[4]);
    // Serial.print("  c5: "); Serial.print(canMsg.data[5]);
    // Serial.print("  c6: "); Serial.print(canMsg.data[6]);
    // Serial.print("  ly: "); Serial.print(canMsg.data[7]);
    // Serial.print("  lx: "); Serial.print(PS4.LStickX());
    // Serial.print("  rx: "); Serial.print(PS4.RStickX());
    // Serial.print("  ly: "); Serial.print(PS4.LStickY());
    // Serial.print("  ry: "); Serial.print(PS4.RStickY());
    // Serial.print("  lxint: "); Serial.print(lx);
    // Serial.print("  lyint: "); Serial.print(ly);
    Serial.println();

    if (PS4.PSButton() == 1){
      if (currentTick - nowTick > 500) {
			  mode = mode*-1;
			  nowTick = currentTick;
		  }
    }
    if (mode==1) {
      if (currentTick1 - nowTick1 > 100) {
			  PS4.setLed(255, 102, 204);
        PS4.sendToController();
			  nowTick1 = currentTick1;
		  }
    }
	  else if (mode==-1) {
      if (currentTick1 - nowTick1 > 100) {
			  PS4.setLed(0, 255, 255); 
        PS4.sendToController();
			  nowTick1 = currentTick1;
		  }
    }

    Serial.print("  mode: "); Serial.print(mode); 
    Serial.println();
    Serial.print("  PS: "); Serial.print(PS4.PSButton());   

    canMsg.data[0] &= ~(0xFF); // Mask 0xFF = 11111111, mengatur kembali semua bit ke 0
    canMsg.data[1] &= ~(0xFF); // Mask 0xFF = 11111111, mengatur kembali semua bit ke 0
    canMsg.data[2] &= ~(0xFF); // Mask 0xFF = 11111111, mengatur kembali semua bit ke 0
    canMsg.data[3] &= ~(0xFF); // Mask 0xFF = 11111111, mengatur kembali semua bit ke 0
  }
}