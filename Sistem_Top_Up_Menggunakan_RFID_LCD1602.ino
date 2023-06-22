/*
    Software Mini Bank ATM dengan Sistem RFID dan Internet of Things (Blynk)
    Arduino Mega2560 dan ESP8266 (Komunikasi Serial RX TX)
*/

/*  -------------------------------------------------- LIBRARY yang digunakan -------------------------------------------- */
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include "Adafruit_Thermal.h"

/*  -------------------------------------------------- DEKLARASI Pin yang digunakan -------------------------------------------- */
#define pinSDA 10
#define pinRST 8

#define RX_Pin 4
#define TX_Pin 3

#define RX_ESP8266 6
#define TX_ESP8266 7

/*  -------------------------------------------------- OBJECT Library yang digunakan -------------------------------------------- */
MFRC522 RFID(pinSDA, pinRST);
LiquidCrystal_I2C lcd(0x27 , 16 , 2);
SoftwareSerial mySerial(14, 15);
SoftwareSerial Send_ESP8266(RX_ESP8266 , TX_ESP8266);
SoftwareSerial mySerial_Print(RX_Pin, TX_Pin);

Adafruit_Thermal printer(&mySerial_Print);

/*  -------------------------------------------------- VARIABLE yang digunakan -------------------------------------------- */
const byte ROWS = 4; //BARIS
const byte COLS = 4; //KOLOM

/*  -------------------------------------------------- TATA LETAK Keypad -------------------------------------------- */
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {22, 23, 24, 25};
byte colPins[COLS] = {26, 27, 28, 29};

Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

/*  -------------------------------------------------- VARIBALE yang digunakan -------------------------------------------- */
char customKey;         //Variabel penampung input keypad
int number = 0;         //Variabel penampung nilai angka

int c = 1,
    cek = 1,
    loopp  = 1;

int index;

String Nama[] = {"NULL", "BAGAS", "ALVIN DYLAN", "GALIH  DWI"};
String ID_RFID[] = {"NULL", "52 4B 6E 22", "6B 48 F3 51", "0B 8C 0A 51"};
unsigned int ID_Password[] = {0000, 1234, 1234, 1234};
int Saldo[] = {0, 0, 0, 0};

int angka_saldo;
char Pilih;

int Tagihan, Topup;

String content = "";
byte letter;

bool ACC_Kartu = false;
bool Input_Password = false;
bool Pilih_Menu = false;
bool Input_Tagihan = false;
bool Input_Topup = false;

unsigned long hitungan_milis;
unsigned long milis_sekarang;
const unsigned long nilai = 1000;

/*  -------------------------------------------------- PROGRAM Void SETUP (Dibaca 1x Ketika Arduino Awal Hidup) -------------------------------------------- */
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Send_ESP8266.begin(9600);
  mySerial_Print.begin(9600);
  EEPROM.begin();
  SPI.begin();
  lcd.init();
  lcd.backlight();
  RFID.PCD_Init();
  printer.begin(9600);
  mp3_set_serial(mySerial);
  mp3_set_volume(100);
  Serial.println("RFID READER");
  Serial.println("");
  Serial.println("Tap Kartu/Gantungan !");
  Serial.println();
  Saldo[1] = EEPROM.read(1);
  Saldo[2] = EEPROM.read(2);
  Saldo[3] = EEPROM.read(3);
  hitungan_milis = 0;
}

/*  -------------------------------------------------- PROGRAM TAMPILAN AWAL pada LCD  -------------------------------------------- */
void Tampilan_Awal() {
  while (loopp <= 1) {
    lcd.setCursor(0, 0);
    lcd.print(" Tap Kartu Anda!  ");
    Send_ESP8266.print("#" + String(1234) + "&");
    Serial.print("\n");
    Serial.print("#" + String(1234) + "&");
    loopp++;
  }
}

/*  -------------------------------------------------- PROGRAM PILIHAN MENU pada LCD -------------------------------------------- */
void Menu() {
Pilih_Menu :
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1. Bayar (A)");
  lcd.setCursor(0, 1);
  lcd.print("2. Top Up (B)");
}

/*  -------------------------------------------------- PROGRAM MENU TAGIHAN -------------------------------------------- */
void Menu_Tagihan()
{
  Pilih_Menu = false;
  Input_Tagihan = true;
  lcd.clear();
  while (Input_Tagihan) {
    lcd.setCursor(0, 0);
    lcd.print("Tagihan :");              //Tampilan pada layar LCD
    customKey = customKeypad.getKey();    //Baca input keypad


    //------------Prosedur jika input berupa angka------------//
    switch (customKey) {
      case '0' ... '9':
        c = c + 1;
        lcd.setCursor((9 + (c - 1)), 0);
        Tagihan = Tagihan * 10 + (customKey - '0');
        lcd.print(customKey);
        lcd.print(".000");
        break;

      //------------Jika input '#' maka cek password------------//
      case '#':
        Saldo[index] = Saldo[index] - Tagihan;
        lcd.setCursor(0, 1);
        lcd.print("Saldo   : ");
        lcd.print(Saldo[index]);  //Tampilan LCD
        lcd.print(".000");
        EEPROM.write(index, Saldo[index]);
        int Sisa_Saldo = Saldo[index];
        Send_ESP8266.print("#" + Nama[index] + "?");
        Send_ESP8266.print("#" + String(Sisa_Saldo) + "!");
        Serial.print("\n");
        Serial.print("#" + Nama[index] + "?");
        Serial.print("#" + String(Sisa_Saldo) + "!");

        //Thermal Printer Mencetak Struk
        printer.setSize('M');
        printer.justify('C');
        printer.println(F("RIWAYAT TRANSAKSI\n"));

        printer.setSize('S');
        printer.justify('L');
        printer.println("Tagihan :" + String(Tagihan) + ".000");
        //        printer.print(Tagihan);
        //        printer.print(F(".000"));

        printer.setSize('S');
        printer.justify('L');
        printer.println("Sisa Saldo :" + String(Saldo[index]) + ".000");
        //        printer.print(Saldo[index]);
        //        printer.print(".000");

        printer.setSize('M');
        printer.justify('C');
        printer.println(F("Terima Kasih\n\n"));
        printer.feed(2);

        printer.sleep();      // Tell printer to sleep
        delay(3000L);         // Sleep for 3 seconds
        printer.wake();       // MUST wake() before printing again, even if reset
        printer.setDefault();

        delay(2000);
        lcd.setCursor(0, 2);
        lcd.print("SUKSES");
        mp3_play (1);
        delay(500);
        lcd.clear();
        c = 1;
        Tagihan = 0;
        Input_Tagihan = false;
        cek = 0;
        loopp = 1;
        break;

      case '*':
        Tagihan = 0;
        c = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Tagihan :");
        break;
    }
  }
}

/*  -------------------------------------------------- PROGRAM MENU TOP UP -------------------------------------------- */
void Menu_Topup()
{
  Pilih_Menu = false;
  Input_Topup = true;
  lcd.clear();

  while (Input_Topup) {
    lcd.setCursor(0, 0);
    lcd.print("Topup  :");              //Tampilan pada layar LCD
    customKey = customKeypad.getKey();    //Baca input keypad

    //------------Prosedur jika input berupa angka------------//
    switch (customKey) {
      case '0' ... '9':
        c = c + 1;
        lcd.setCursor((8 + (c - 1)), 0);
        Topup = Topup * 10 + (customKey - '0');
        lcd.print(customKey);
        lcd.print(".000");
        break;

      //------------Jika input '#' maka cek password------------//
      case '#':
        Saldo[index] = Saldo[index] + Topup;
        lcd.setCursor(0, 1);
        lcd.print("Saldo  : ");
        lcd.print(Saldo[index]);  //Tampilan LCD
        lcd.print(".000");
        EEPROM.write(index, Saldo[index]);
        int Sisa_Saldo = Saldo[index];
        Send_ESP8266.print("#" + Nama[index] + "?");
        Send_ESP8266.print("#" + String(Sisa_Saldo) + "!");
        Serial.print("\n");
        Serial.print("#" + Nama[index] + "?");
        Serial.print("#" + String(Sisa_Saldo) + "!");

        //Thermal Printer Mencetak Struk
        printer.setSize('M');
        printer.justify('C');
        printer.print(F("RIWAYAT TRANSAKSI\n"));

        printer.setSize('S');
        printer.justify('L');
        printer.println("Topup :" + String(Topup) + "000");
        //        printer.print(Topup);
        //        printer.print(F(".000"));

        printer.setSize('S');
        printer.justify('L');
        printer.println("Sisa Saldo :" + String(Saldo[index]) + ".000");
        //        printer.print(Saldo[index]);
        //        printer.print(".000");

        printer.setSize('M');
        printer.justify('C');
        printer.println(F("Terima Kasih\n\n"));
        printer.feed(2);

        printer.sleep();      // Tell printer to sleep
        delay(3000L);         // Sleep for 3 seconds
        printer.wake();       // MUST wake() before printing again, even if reset
        printer.setDefault();

        delay(2000);
        lcd.setCursor(0, 2);
        lcd.print("SUKSES");
        mp3_play (1);
        delay(500);
        lcd.clear();
        c = 1;
        Topup = 0;
        Input_Topup = false;
        cek = 0;
        loopp = 1;
        break;

      case '*':
        Topup = 0;
        c = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Topup  :");
        break;
    }
  }
}

/*  -------------------------------------------------- PROGRAM INPUT PASSWORD -------------------------------------------- */
void Masuk_Password() {
  lcd.setCursor(0, 0);
  lcd.print("Kartu Diterima");
  lcd.setCursor(0, 1);
  lcd.print("Password :");              //Tampilan pada layar LCD
  customKey = customKeypad.getKey();    //Baca input keypad


  //------------Prosedur jika input berupa angka------------//
  switch (customKey) {
    case '0' ... '9':

      /* ------------Banyak Password di LCD------------ */
      c = c + 1;
      lcd.setCursor((9 + (c - 1)), 1);
      number = number * 10 + (customKey - '0');
      lcd.print("*");
      break;

    /* ------------Jika input '#' maka cek password------------ */
    case '#':
      if (number == ID_Password[index]) {    /* Jika password benar, maka Kartu Diterima */
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("Akses Kartu Diterima");  /* Tampilan LCD Akses Diterima  */
        delay(2000);
        number = 0;
        c = 1;

        Input_Password = false;
        Pilih_Menu = true;
        lcd.clear();

        while (Pilih_Menu) {
          lcd.setCursor(0, 0);
          lcd.print("1. Bayar (A)");
          lcd.setCursor(0, 1);
          lcd.print("2. Top Up (B)");

          customKey = customKeypad.getKey();    //Baca input keypad

          /*  -------------------------------------------------- Jika A ditekan, maka masuk Menu Tagihan -------------------------------------------- */
          if (customKey == 'A')
          {
            Menu_Tagihan();
          }
          /*  -------------------------------------------------- Jika B ditekan, maka masuk Menu Top Up -------------------------------------------- */
          if (customKey == 'B')
          {
            Menu_Topup();
          }
        }
      }


      else {
        lcd.clear();//Jika salah, maka
        lcd.setCursor(0, 1);
        lcd.print("Password Salah");  //Tampilan LCD
        delay(2000);
        number = 0;
        c = 1;
        lcd.clear();
      }
      break;

    //------------Jika input '*' maka hapus tampilan------------//
    case '*':
      number = 0;
      c = 1;
      lcd.clear();
      if (ACC_Kartu = true)
      {
        lcd.setCursor(0, 1);
        lcd.print("Kartu Diterima");  //Tampilan LCD
      }
      break;
  }
}

/*  -------------------------------------------------- PROGRAM RFID READER untuk membaca kartu RFID-------------------------------------------- */
void baca_RFID() {
  if ( ! RFID.PICC_IsNewCardPresent()) {
    return;
  }
  if ( ! RFID.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("ID Tag :");
  content = "";

  for (byte i = 0; i < RFID.uid.size; i++) {
    Serial.print(RFID.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(RFID.uid.uidByte[i], HEX);
    content.concat(String(RFID.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(RFID.uid.uidByte[i], HEX));
  }

  content.toUpperCase();

  for (cek ; cek <= 10; cek++)
  {
    if (content.substring(1) == ID_RFID[cek]) {
      ACC_Kartu = true;
      index =  cek;
    }
  }

  /*  -------------------------------------------------- Jika Kartu RFID Terdaftar Maka Izinkan Masuk Menu -------------------------------------------- */
  if (content.substring(1) == ID_RFID[index] )
  {
    lcd.clear();
    Serial.println("\t Card Authorized Access");
    Serial.println();

    Input_Password = true;
    while (Input_Password)
    {
      Masuk_Password();
    }
    delay(500);
  }
  Kartu_Ditolak();
}

/*  -------------------------------------------------- PROGRAM KARTU DITOLAK (Belum Terdaftar) -------------------------------------------- */
void Kartu_Ditolak()
{
  if (content.substring(1) != ID_RFID[index]) {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Kartu Ditolak");
    Serial.println("\tCard Declined");
    delay(1000);
    cek = 0;
    loopp = 1;
    Tampilan_Awal();
    lcd.setCursor(0, 1);
    lcd.print("                  ");
    baca_RFID();
  }
}


void loop() {
  Tampilan_Awal();
  baca_RFID();

}
