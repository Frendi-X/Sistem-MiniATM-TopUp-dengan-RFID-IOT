#define BLYNK_TEMPLATE_ID "TMPLTVEAzfuM"
#define BLYNK_TEMPLATE_NAME "Pembatas Komsumtif Siswa Ponorogo"
#define BLYNK_AUTH_TOKEN "dP3-4UJdx2P7s3Yi6EGKt6Yf01T2xY1k"  //REOG ROBOTIC AUTH
#define BLYNK_PRINT Serial

/* ------------------------------------------------------------------ HEADER FILE ATAU LIBRARY */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include<SoftwareSerial.h>

/* ------------------------------------------------------------------ KOMUNIKASI SERIAL RX TX di PIN D1, D2 */
SoftwareSerial mySerial(D2, D1);

/* ------------------------------------------------------------------ WIDGET LCD BLYNK MENGGUNAKAN VIRTUAL V0 */
WidgetLCD lcd(V0);
BlynkTimer timer;

/* ------------------------------------------------------------------ KONEKSI WIFI DAN AUTH */
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "AndroidAP";
char pass[] = "12345678";

/* ------------------------------------------------------------------ VARIABLE DATA SUHU DAN NTU */
int Sisa_Saldo;

/* ------------------------------------------------------------------ VARIABLE PEMPROSESAN DATA YANG DITERIMA DARI ARDUINO */
String data, Nama;
char CharData;
String StringData, dataSubs;
int index1, index2;

/* ------------------------------------------------------------------ PROSES MENYAMBUNG WIFI */
void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to AP");
}

/* ------------------------------------------------------------------ KIRIM DATA SENSOR KE VIRTUAL BLYNK */
void sendSensor()
{
  Blynk.virtualWrite(V1, Sisa_Saldo);
}

/* ------------------------------------------------------------------ VOID SETUP() */
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  InitWiFi();
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(1000L, sendSensor);
  lcd.print(0, 0, "    Kalpetar    ");
}

/* ------------------------------------------------------------------ VOID LOOP() */
void loop() {

  /* BACA DATA DARI ARDUINO */
  while (mySerial.available() > 0)
  {
    delay(10);
    CharData = mySerial.read();
    StringData += CharData;

    /* PARSING DATA SISA SALDO */
    if (StringData.length() > 0 && CharData == '?')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('?', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";
      
      Nama = dataSubs;
      Serial.print("Nama : ");
      Serial.println(Nama);
      lcd.clear();
      lcd.print(0, 0, Nama);
    }

    else if (StringData.length() > 0 && CharData == '!')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('!', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data = atof(buf);

      /* DATA SUHU */
      Sisa_Saldo = Data * 1000;
      Serial.print("Sisa Saldo : ");
      Serial.println(Sisa_Saldo);
      lcd.print(0, 1, String(Sisa_Saldo));
      Blynk.email("reog.robotic@gmail.com", "SmartHome Respon", "Relay 2 OFF");
      Blynk.logEvent("transaksi_alert" , Nama + " " + String(Sisa_Saldo));
    }

    if (StringData.length() > 0 && CharData == '&')
    {
      index1 = StringData.indexOf('#');
      index2 = StringData.indexOf('&', index1 + 1);
      dataSubs = StringData.substring(index1 + 1, index2);
      StringData = "";

      //Mengubah Data String ke Float
      char buf[dataSubs.length()];
      dataSubs.toCharArray(buf, dataSubs.length() + 1);
      float Data = atof(buf);

      float CLEAR = Data;
      Serial.print("Clear : ");
      Serial.println(CLEAR);
      lcd.clear();
      lcd.print(0, 0, "    Kalpetar    ");
    }
  }

  /* JALANKAN BLYNK */
  Blynk.run();
  /* JALANKAN TIMER BLYNK */
  timer.run();
}
