#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(D8, D7); // RX, TX

String docType = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
String webpage = "<html> <head> <meta charset='utf-8'> <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js'></script> <title>IoT</title> <style> body{ background-color: #323232; } input.switch{ font-size: 64px; padding: 32px; margin: 16px; background-color: #424242; width: 60%; color: #ffffff; } h1{ font-size: 90px; color: #ffffff; margin: auto; } div{ text-align: center; } hr{ width: 70%; } input.slider{ width: 60%; } p{ font-size: 32px; color: white; } </style> </head> <body> <div class='toggle-secction'> <h1>Switch</h1> <input type='button' value='D4: OFF' class='switch' id='D4' onmousedown='sendGpio(this.id)'> <hr> </div> <div class='slider-secction'> <h1>Slider</h1> <input type='range' class='slider' id='pwmIn' value='0' min='0' max='255'> <p>Value: <span id='pwmInText'></span></p> <script type='text/javascript'> gpioState = false; var slider = document.getElementById('pwmIn'); var sliderText = document.getElementById('pwmInText'); var gpio = document.getElementById('D4'); sliderText.innerHTML = slider.value; $.ajaxSetup({timeout:1000}); function sendGpio(id){ gpioState = !gpioState; if (gpioState){ $.get('/?gpio=1&'); gpio.value = id + ': ON'; } else{ $.get('/?gpio=0&'); gpio.value = id + ': OFF'; } } slider.oninput = function(){ sliderText.innerHTML = slider.value; $.get('/?value='+slider.value+'&'); {Connection: close}; } </script> <hr> </div> </body></html>";

const byte led = LED_BUILTIN; // D4
const byte pwm[1] = {D5};     // D2, D5, D6, D8 | [0 - 1023]
const byte lamp[1] = {D3};    // D0, D1, D3, D4
byte val = 0;
int sliderVal = 0;

const char* ssid     = "INFINITUM2372_2.4";
const char* password = "uH0L0Ybioa";

//const char* ssid     = "3Com";
//const char* password = "";

//const char* ssid     = "INFINITUM5413_2.4";
//const char* password = "r4raEWFv0P";

//Static IP address configuration 
// static:  192.168.1.77
// gateway: 192.168.1.254
IPAddress staticIP(192, 168, 1, 77);  //ESP static ip
IPAddress gateway(192, 168, 1, 254);  //Router (Gateway)
IPAddress subnet(255, 255, 255, 0);   //Subnet mask
IPAddress dns(8, 8, 8, 8);            //DNS

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

  // GPIO's
  pinMode(led, OUTPUT);
  pinMode(pwm[0], OUTPUT);
  pinMode(lamp[0], OUTPUT);

  // BUILDINLED: OFF
  digitalWrite(led, HIGH);

  // Connect to WiFi network
  Serial.print(F("\nConnecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  WiFi.config(staticIP, gateway, subnet, dns);
  Serial.println(F("\nWiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());

  // BUILDINLED: ON
  digitalWrite(led, LOW);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(request);

  // Match the request
  if (request.indexOf("/?gpio=")>=0){
    val = getValue(request).toInt();
    Serial.println(val);
    digitalWrite(pwm[0], val);
    mySerial.write("DIGITAL\n");
  } else if (request.indexOf("/?value=")>=0){
    sliderVal = getValue(request).toInt();
    sliderVal = map(sliderVal, 0, 255, 0, 1023);
    analogWrite(pwm[0], sliderVal);
    mySerial.write("ANALOG\n");
  }
  /*else {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  } */

  // Set LED according to the request
  // ...

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(docType);
  client.print(webpage);
  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
  client.stop();
  Serial.println("Client disconnected.");
}

String getValue(String req){
  int posA, posB;
  posA = req.indexOf("=");
  posB = req.indexOf("&");
  return req.substring(posA+1, posB);
}
