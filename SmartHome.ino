#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <IRremote.hpp>
#include <Arduino.h>



const char *ssid = "Room8"; 
const char *password = "Arduino8";
const int dhtSensorPort = 23;
const int touch_pin = 4;
const int light_pin = 2; 
const int touch_threshold = 30;
const int trigPin = 5;   // Trigger pin
const int echoPin = 13;  // Echo pin
const int piezoPin = 18;
const int recvPin = 14;
const int redPin = 27;
const int bluePin = 16;
const int greenPin = 17;

const int button0 = 22;
const int button1 = 12;
const int button2 = 24;
const int button3 = 94;
const int button4 = 8;
const int button5 = 28;
const int button6 = 90;
const int button7 = 66;
const int button8 = 82;
const int button9 = 74;
const int irProtocol = 8;


long duration;
float distanceCm;
bool AlarmOn = false;
bool touched = false;
bool alarmTriggered = false;


WebServer server(80);
DHT dht(dhtSensorPort, DHT22);

void handleRoot() {
 char msg[2000];  // Increased size for added content

  const char* alarmStatus = AlarmOn ? "ON" : "OFF";
  const char* alarmColor = AlarmOn ? "#00cc44" : "#999999";
  const char* alarmTriggeredColor = alarmTriggered ? "#FF0000" : "#00cc44";
  const char* alarmTriggeredStatus = alarmTriggered ? "Yes" : "No";
  snprintf(msg, 1500,
           "<html>\
  <head>\
    <meta http-equiv='refresh' content='4'/>\
    <meta name='viewport' content='width=device-width, initial-scale=1'>\
    <link rel='stylesheet' href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity='sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr' crossorigin='anonymous'>\
    <title>ESP32 DHT Server</title>\
    <style>\
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center;}\
    h2 { font-size: 3.0rem; }\
    p { font-size: 3.0rem; }\
    .units { font-size: 1.2rem; }\
    .dht-labels{ font-size: 1.5rem; vertical-align:middle; padding-bottom: 15px;}\
    </style>\
  </head>\
  <body>\
      <h2>Ture's ESP32 DHT Server!</h2>\
      <p>\
        <i class='fas fa-thermometer-half' style='color:#ca3517;'></i>\
        <span class='dht-labels'>Temperature</span>\
        <span>%.2f</span>\
        <sup class='units'>&deg;C</sup>\
      </p>\
      <p>\
        <i class='fas fa-tint' style='color:#00add6;'></i>\
        <span class='dht-labels'>Humidity</span>\
        <span>%.2f</span>\
        <sup class='units'>&percnt;</sup>\
      </p>\
      <p>\
        <i class='fas fa-lightbulb' style='color:%s;'></i>\
        <span class='dht-labels'>Alarm Status</span>\
        <span>%s</span>\
      </p>\
      <p>\
        <i class='fas fa-lightbulb' style='color:%s;'></i>\
        <span class='dht-labels'>Alarm Triggered Status</span>\
        <span>%s</span>\
      </p>\
  </body>\
</html>",
           readDHTTemperature(), readDHTHumidity(), alarmColor, alarmStatus, alarmTriggeredColor, alarmTriggeredStatus 
          );
  server.send(200, "text/html", msg);
}

void setup(void) {

  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);

  server.begin();
  Serial.println("HTTP server started");

  pinMode(light_pin, OUTPUT);
  digitalWrite(light_pin, LOW); // start with light off
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(piezoPin, OUTPUT);
  
  

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  IrReceiver.begin(recvPin, ENABLE_LED_FEEDBACK);  // Start receiver
  Serial.println("IR Receiver Ready!");


  
 setColor(256, 256, 256);

}

void loop(void) {


  

  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
  int touchValue = touchRead(touch_pin);

  if (touchValue < touch_threshold && !touched) {
    touched = true;             // Debounce: mark touch started
    AlarmOn = !AlarmOn;         // Toggle light state
    alarmTriggered = false;
    digitalWrite(light_pin, AlarmOn ? HIGH : LOW);
    Serial.println(AlarmOn ? "Alarm ON" : "Alarm OFF");
  } else if (touchValue >= touch_threshold && touched) {
    touched = false;            // Reset when finger lifted
  }


  if (AlarmOn == true) {
   // Distance Sensor Part (ALARM)
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Send 10us pulse to trigger
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read echo time
  duration = pulseIn(echoPin, HIGH);
  // Convert to distance
  distanceCm = duration * 0.034 / 2;  // speed of sound = 343 m/s = 0.034 cm/us
  if ((distanceCm < 10) && (distanceCm > 2)) {
    alarmTriggered = true;
    Serial.println("Distance");
    Serial.println(distanceCm);
    Serial.println("Alarm Triggered");
  }


  if (alarmTriggered == true) {
    tone(piezoPin, 1000);
  }

  }


  if (alarmTriggered == false) {
    noTone(piezoPin);
  }

  if (IrReceiver.decode()) {


  uint32_t raw = IrReceiver.decodedIRData.decodedRawData;

  if (raw != 0) {
    Serial.print("IR Data: ");
    Serial.println(raw, HEX);
    
    if (IrReceiver.decodedIRData.protocol == irProtocol) {
      switch (IrReceiver.decodedIRData.command) {
        case button0:
          Serial.println("Setting LED to Off");
          setColor(0, 0, 0);
          break;
        case button1: 
          Serial.println("Setting LED to RED");
          setColor(255, 0, 0);
          break;
        case button2:
          Serial.println("setting LED to Green");
          setColor(0, 255, 0);
          break;
        case button3:
          Serial.println("Setting LED to Blue");
          setColor(0, 0, 255);
          break;
        case button4:
          Serial.println("Setting LED to orange");
          setColor(255, 128, 0);
          break;
        case button5:
          Serial.println("Setting LED to purple");
          setColor(153, 51, 255);
          break;
        case button6:
          Serial.println("Setting LED to pink");
          setColor(255, 0, 255);
          break;
        case button7:
          Serial.println("Setting LED to yellow");
          setColor(255, 255, 0);
          break;
        case button8:
          Serial.println("Setting LED to white");
          setColor(255, 255, 255);
          break;
      }
    }

    /*if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
      Serial.print("Protocol: ");
      Serial.println(IrReceiver.decodedIRData.protocol);
      Serial.print("Command: ");
      Serial.println(IrReceiver.decodedIRData.command);
    }*/

  }

  IrReceiver.resume(); // Always resume after decode
}




}


float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    return t;
  }
}


float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  }
  else {
    return h;
  }
}


void setColor(int R, int G, int B) {
    Serial.println("Trying to run RGB");
    digitalWrite(redPin, R);
    analogWrite(greenPin, G);
    analogWrite(bluePin, B);
  

}

