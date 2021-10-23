#include <TM74HC595Display.h>

#include <MFRC522.h>

#include "RTClib.h"

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above


MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

const int SCLK = 7, RCLK = 6, DIO = 5;

TM74HC595Display disp(SCLK, RCLK, DIO);

DateTime zerotime (2019, 1, 1, 0, 0, 0);


MFRC522::MIFARE_Key key;
void setup() {
  SPI.begin();
  Serial.begin(9600);
  mfrc522.PCD_Init();                                              // Init MFRC522 card

  key.keyByte[0] = 0xA7;
  key.keyByte[1] = 0x3F;
  key.keyByte[2] = 0x5D;
  key.keyByte[3] = 0xC1;
  key.keyByte[4] = 0xD3;
  key.keyByte[5] = 0x33;

}

const byte block = 33, block2 = 32;
const  byte len = 18;

MFRC522::StatusCode status;

byte buffer1[18];

int last_bal = 0;
int32_t last_date = 0;
void loop() {

  disp.send(0xC0, 0b1111);

  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block2, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
    if (status != MFRC522::STATUS_OK) {
      Serial.println("reading error");
      delay(1000);
      return;
    }

    status = mfrc522.MIFARE_Read(block, buffer1, &len);
    if (status != MFRC522::STATUS_OK) {

      Serial.println("reading error");
      delay(1000);
      return;
    }

    Serial.print("Dump of block 32: ");
    for (int i = 0; i < 18; ++i) {
      if (buffer1[i] < 16)Serial.print("0");
      Serial.print(buffer1[i], HEX);
      Serial.print(" ");
    }
    Serial.println();


    Serial.print("Balance: ");
    last_bal = code_to_rubs(buffer1[4], buffer1[5], buffer1[6]);
    last_date = 0 | ((uint32_t)buffer1[0]<<16) | (buffer1[1] << 8) | buffer1[2];
    TimeSpan tri0(last_date * 30 - 86400);
    DateTime tri = zerotime + tri0;
    Serial.print(last_bal);
    Serial.println(" rub.");
    Serial.print("time: ");
    Serial.print(tri.hour());
    Serial.print(':');
    Serial.println(tri.minute());
    Serial.print("date: ");
    Serial.print(tri.day());
    Serial.print('.');
    Serial.print(tri.month());
    Serial.print('.');
    Serial.println(tri.year());
    disp.digit4showZero(last_bal, 3000);
    
    disp.digit4showZero(tri.hour()*100+tri.minute(), 2000);
    disp.digit4showZero(tri.day()*100+tri.month(), 2000);
    disp.digit4showZero(tri.year(), 2000);
    disp.send(0xEF, 0b1111);
  }


  delay(500); //change value if you want to read cards faster
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

int code_to_rubs(int h, int m, int l)
{
  int res = 0;
  if (h) {
    res = h % 16;
    h /= 16;
    res += (h % 16) * 10;
    res *= 100;
  }
  if (m) {
    res = m % 16;
    m /= 16;
    res += (m % 16) * 10;
    res *= 10;
  }
  l /= 16;
  res += (l % 16);
  return res;
}
