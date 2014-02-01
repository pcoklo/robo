#include <Wire.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>

YunServer server;

int blinkPin, fadePin, blinkDelay, i;
int brightness = 0, fadeAmount = 1;
boolean ledState = 0, fade = 0, blink = 0;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  Bridge.begin();
  Wire.begin();
  server.listenOnLocalhost();
  server.begin();
  digitalWrite(13, LOW);
}

void loop() {

  if(i<0)i=0;
  if(blink && i>=blinkDelay){
    ledState=!ledState;
    digitalWrite(blinkPin,ledState);
    i=0;
  }
  
  if(fade){
    analogWrite(fadePin, brightness);
    brightness = brightness + fadeAmount;
    if (brightness == 0 || brightness == 255){
      fadeAmount = -fadeAmount ;
    }
  }
  YunClient client=server.accept();
  if (client){
    process(client);
    client.stop();
  }
  delay(10);
  i++;
}

void process(YunClient client) {
  fade=0; blink=0;
  String command = client.readStringUntil('/');
  if (command == "digital") {
    digitalCommand(client);
  }
  if (command == "analog") {
    analogCommand(client);
  }
  if (command == "mode") {
    modeCommand(client);
  }
  if (command == "blink") {
    blinkCommand(client);
  }
  if (command == "fade") {
    fadeCommand(client);
  }
  if (command == "wire") {
    wireCommand(client);
  }
}

void digitalCommand(YunClient client){
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/'){
    value = client.parseInt();
    digitalWrite(pin, value);
  }
  else value = digitalRead(pin);
  client.print(F("Pin D"));
  client.print(pin);
  client.print(F(" set to "));
  client.println(value);
}

void analogCommand(YunClient client){
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/'){
    value = client.parseInt();
    analogWrite(pin, value);
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" set to analog "));
    client.println(value);
  }
  else{
    value = analogRead(pin);
    client.print(F("Pin A"));
    client.print(pin);
    client.print(F(" reads analog "));
    client.println(value);
  }
}

void modeCommand(YunClient client) {
  int pin;
  pin = client.parseInt();
  if (client.read() != '/') {
    client.println(F("error"));
    return;
  }
  String mode = client.readStringUntil('\r');
  if (mode == "input") {
    pinMode(pin, INPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }
  if (mode == "output") {
    pinMode(pin, OUTPUT);
    // Send feedback to client
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }
  client.print(F("error: invalid mode "));
  client.print(mode);
}

void blinkCommand(YunClient client){
  blink=1;
  blinkPin = client.parseInt();
  if (client.read() == '/') {
    blinkDelay = client.parseInt();
  }
  else {
    blinkDelay = 100;
  }
  client.print(F("Pin D"));
  client.print(blinkPin);
  client.println(F(" set to blink"));
}

void fadeCommand(YunClient client){
  fadePin = client.parseInt();
  brightness = 1;
  if(digitalRead(fadePin)) brightness = 254;
  client.print(F("Pin D"));
  client.print(fadePin);
  client.println(F(" set to fade"));
  fade=1;
}

void wireCommand(YunClient client) {
  byte error;
  int nDevices = 0, address = client.parseInt();
  if(address==0){
    client.print(F("Getting all available addresses:"));
    for(int i=1; i<127; i++){
      Wire.beginTransmission(i);
      error = Wire.endTransmission();
      if(error==0){
        if(nDevices==0){
          client.print(F(" "));
          client.print(i);
        }
        else{
          client.print(F(", "));
          client.print(i);
        }
        nDevices++;
      }
    }
    if(nDevices == 0) client.print(F(" No I2C devices found\n"));
    else client.print(F(", done\n"));
  }
  else if(client.read() == '/'){
    String comand = client.readStringUntil('\r');
    byte l=comand.length(), data[l];
    for(byte i=0;i<l;i++) data[i]=comand.charAt(i);
    Wire.beginTransmission(address);
    Wire.write(data,l);
    error = Wire.endTransmission();
    client.print(F("Comand: "));
    client.print(comand);
    client.print(F("\nAddress: "));
    client.println(address);
    if(error!=0)client.println(F("FAILED!"));
  }
}
