#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Malaikat subuh"
#define WIFI_PASSWORD "hanyaberlima"
#define API_KEY "AIzaSyCMrTetOatnKP5BEMy8KfXNY6DOcWPa9Q0"
#define DATABASE_URL "https://caas-cps-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int hasilldr;
int pinrelay = D7;

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

void setup() {
  pinMode(pinrelay,INPUT);
  pinMode(pinrelay,OUTPUT);
  digitalWrite(pinrelay,LOW);
  Serial.begin(9600);

   // initialize LCD
  lcd.begin();
  // turn on LCD backlight                      
  lcd.backlight();

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign up successful");
  } else {
    Serial.printf("Firebase sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback; // Provided by the TokenHelper addon
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  hasilldr = analogRead(A0);
  Serial.println("----Report----");
  Serial.print("Intensitas Cahaya = ");
  Serial.println(hasilldr);

  // Control relay based on light intensity
  if (hasilldr >= 50) {
    digitalWrite(pinrelay, LOW);
    Serial.println("Lampu Dimatikan");
    lcd.setCursor(0, 0);
    lcd.print("Lampu Dimatikan");
  } else if (hasilldr < 45) {
    digitalWrite(pinrelay, HIGH);
    Serial.println("Lampu Dinyalakan");
    lcd.setCursor(0, 0);
    lcd.print("Lampu Dinyalakan");
  }

  // Send data to Firebase
  if (Firebase.ready()) {
    if (Firebase.RTDB.setInt(&fbdo, "sensor/ldr", hasilldr)) {
      Serial.println("Data sent to Firebase successfully!");
    } else {
      Serial.printf("Failed to send data to Firebase: %s\n", fbdo.errorReason().c_str());
    }
  }

  // Update send data time
  sendDataPrevMillis = millis();

  delay(1000);
}
