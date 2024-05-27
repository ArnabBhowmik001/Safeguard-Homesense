#define BLYNK_TEMPLATE_ID "TMPL3uRODl8Kz"
#define BLYNK_TEMPLATE_NAME "SwitchONOFF"
#define BLYNK_AUTH_TOKEN "4aj77kLVaBsc-eZHvsM5ksk96JfvgOmT"
#define BLYNK_PRINT Serial

#include <Firebase_ESP_Client.h>
#include<Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP_Mail_Client.h>
#include "time.h"
//Provide the token generation process info.
#include <addons/TokenHelper.h>
//Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define MQ2_PIN 34 // Analog pin for MQ2 sensor
#define IR_PIN 18   // Digital pin for IR sensor
#define BUZZ_PIN 17 // Digital pin for LED
#define LED_PIN 25
#define SCREEN_ADDRESS 0x27
#define SCREEN_COLUMNS 16
#define SCREEN_ROWS 2
#define SMTP_server "smtp.gmail.com"
#define SMTP_Port 465
#define sender_email "esp082950@gmail.com"
#define sender_password "wmuf gnrl yhzx gtga"
#define Recipient_email "adnanadoo309@gmail.com"
#define Recipient_name "Adnan khawaja syed"
SMTPSession smtp;
Session_Config config;

LiquidCrystal_I2C lcd(0x27, 16, 2); 
// Insert Firebase project API Key
#define API_KEY "AIzaSyDM_5vrVMUGRCRfEVBSQBwB2qpzBecvvo0"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://esp32-b19da-default-rtdb.firebaseio.com/"
char ssid[] = "Galaxy A9 (2018)99F6";
char pass[] = "12345876";
char apiKey[] = "660ecee04484d641560174ryue1d057";
const char* ntpServer = "pool.ntp.org";
/* Callback function to get the Email sending status */
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig f_config;
bool signupOK = false;
 String getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return("");
  }
  char t[200];
  strftime(t,200, "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return String(t);
}
void firebaseInit(){
   f_config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  f_config.database_url = DATABASE_URL;
  /* Sign up */
  if (Firebase.signUp(&f_config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", f_config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  f_config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&f_config,&auth);
  Firebase.reconnectWiFi(true);
}
void sendEmail(String msg){
 smtp.debug(1);
  config.server.host_name = SMTP_server ;
  config.server.port = SMTP_Port;
  config.login.email = sender_email;
  config.login.password = sender_password;
  config.login.user_domain = "";
  config.time.ntp_server = "pool.ntp.org,time.nist.gov";
  config.time.gmt_offset = 5.5;
  config.time.day_light_offset = 0;
  if (!smtp.connect(&config)) {
    Serial.println("Error connecting to SMTP server");
     ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  }
  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }
  /* Declare the message class */

  SMTP_Message message;

  message.sender.name = "ESP 32";

  message.sender.email = sender_email;

  message.subject = "ESP 32 WARNING EVENT";

  message.addRecipient(Recipient_name,Recipient_email);
//  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
//  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;




 //Send simple text message

  String textMsg = msg;

  message.text.content = textMsg.c_str();

  message.text.charSet = "us-ascii";

  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit ;

  if (!MailClient.sendMail(&smtp, &message)){
     ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    Serial.println("Error sending Email, " + smtp.errorReason());}

  }
void sendLocationNotification(String ipAddress,String event) {
  HTTPClient http;
  String apiUrl = "http://ip-api.com/json/" + ipAddress;

  http.begin(apiUrl);
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    // Parsing JSON
    String lati=doc["lat"];
    String lon=doc["lon"];
    String regionName=doc["regionName"];
    String city=doc["city"];
    String zip=doc["zip"];
    String country=doc["country"];
    String msg="Alert! "+event+" detected at:"+" Latitude:"+lati+", Longitude: "+lon+", city: "+city+", state:"+regionName+", country:"+country+", zip code:"+zip;
    Blynk.logEvent("event_detected",msg);
    sendEmail(msg);

  } else {
    Serial.println("Error getting location data");
  }

  http.end();
}
void sendNotification(String event) {
  
 String apiUrl = "http://api.ipify.org";
 
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String payload = "";

    http.begin(client, apiUrl);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      Serial.print("the server provided this text : ");
      Serial.println(payload);
      sendLocationNotification(payload,event);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

}

void connectWiFi() {
  WiFi.begin(ssid,pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}
void detectGas() {
  int gasValue = analogRead(MQ2_PIN);
  gasValue = map(gasValue, 0, 4095, 0, 100);
//  Serial.println(gasValue);
  Blynk.virtualWrite(V0, gasValue); // Update Blynk app with gas sensor value
  if (gasValue > 50) {
    digitalWrite(BUZZ_PIN , HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gas detected!");
    delay(1000);
    digitalWrite(BUZZ_PIN , LOW);
     if (Firebase.ready() && signupOK){

  // Save data to Firebase

  FirebaseJson json;
  json.add("value", gasValue);
  json.add("TimeStamp",getTime());
  // Push data to Firebase database
  if (Firebase.RTDB.pushJSON(&fbdo,"test/gas_sensor",&json)) {
    Serial.println("Data saved to Firebase!");
  } else {
    Serial.println("Failed to save data to Firebase.");
    Serial.println("Error: " + fbdo.errorReason());
  }
      }
    sendNotification("Gas");
    delay(2000);
    lcd.clear();
    
  } else {
    digitalWrite(BUZZ_PIN , LOW);
  }
}

void detectIR() {
  int irValue = digitalRead(IR_PIN);
  Blynk.virtualWrite(V1, irValue); // Update Blynk app with IR sensor value
  if (irValue == LOW) {
    digitalWrite(BUZZ_PIN , HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Intruder detected!");
    delay(1000);
    digitalWrite(BUZZ_PIN , LOW);
     if (Firebase.ready() && signupOK){


  // Save data to Firebase

  FirebaseJson json;
  json.add("value", irValue);
  json.add("TimeStamp",getTime());
  // Push data to Firebase database
  if (Firebase.RTDB.pushJSON(&fbdo, "test/ir_sensor",&json)) {
    Serial.println("Data saved to Firebase!");
  } else {
    Serial.println("Failed to save data to Firebase.");
    Serial.println("Error: " + fbdo.errorReason());
  }
      }
      
    sendNotification("Intruder");
    delay(2000);
    digitalWrite(BUZZ_PIN , LOW);
  } else {
    digitalWrite(BUZZ_PIN , LOW);
  }
}
void switchLight(){
  if(Firebase.RTDB.getInt(&fbdo, "test/switch")){
//      Serial.println(fbdo.intData());
   if(fbdo.dataType()== "int"){
    int read_value=fbdo.intData();
    if(read_value==1){
      digitalWrite(LED_PIN , HIGH);
    }else{
      digitalWrite(LED_PIN , LOW);
    }
    }else{
      Serial.println(fbdo.errorReason());
    }
  }
}

void setup() {
  Serial.begin(9600);
  connectWiFi();
  configTime(19800, 0, ntpServer);
  firebaseInit();
  Blynk.begin("4aj77kLVaBsc-eZHvsM5ksk96JfvgOmT",ssid,pass);
  pinMode(IR_PIN, INPUT);
  pinMode(MQ2_PIN,INPUT);
  pinMode(BUZZ_PIN , OUTPUT);
  pinMode(LED_PIN , OUTPUT);
  digitalWrite(BUZZ_PIN , LOW);
  switchLight();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  
}

void loop() {
 
    switchLight();
    detectGas();
    detectIR();
    Blynk.run();

}