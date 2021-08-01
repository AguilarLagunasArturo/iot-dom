/* 
 *  Autor: Arturo Aguilar Lagunas
 *  Descripcion: IoT Device (Prototipo)
*/

/* Librerias */
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

/* Constantes */
#define LED_PIN         4           // GPIO para rele que controla el dispositivo
#define SW_PIN          16          // GPIO para switch que controla el dispositivo
#define BAUD_RATE       115200      // Valor por defecto para NodeMCU
#define EEROM_SPACE     512         // Espacio en EEROM
#define HTTPS_PORT      443         // HTTPS Port
#define URL             "https://km83gb3bwa.execute-api.us-east-1.amazonaws.com/default/iot-skill-api"
#define GET_STATE       "{\"action\": \"get-device-state\", \"device-id\": \"iot-skill-01\"}"
#define SET_CONNECTION  "{\"action\": \"set-device-connection\", \"device-id\": \"iot-skill-01\"}"
#define SET_ON          "{\"action\": \"set-device-state\", \"device-id\": \"iot-skill-01\", \"value\": \"ON\"}"
#define SET_OFF         "{\"action\": \"set-device-state\", \"device-id\": \"iot-skill-01\", \"value\": \"OFF\"}"

/* Variables */
bool sw_state = true;                // Conserva el estado actual del switch físico
bool sw_last_state = true;           // Conserva el ultimo estado del switch físico
bool sw_toggle = false;              // Conserva el estado actual del switch digital y físico
bool sw_digital_state = false;       // Conserva el estado actual del switch digital
bool sw_digital_last_state = false;  // Conserva el ultimo estado del switch digital

unsigned long int t = 0;             // Tiempo actual del programa
unsigned long int dt_state = 0;      // Desfase entre el tiempo actual y la ultima actualizacion del dispositivo
unsigned long int dt_bounce = 0;     // Desfase entre el tiempo actual y el ultimo debounce del dispositivo
int time_gap_state = 5000;           // Tiempo de espera para actualizar estado del dispositivo
int time_gap_bounce = 100;           // Tiempo de espera para debounce

bool saved = false;
String ssid = "";
String pass = "";

/* Internet */
const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
WiFiClientSecure clientSecure;
HTTPClient httpClient;

/* Funciones */
void writer(String msg, int pos = 0){
  int addr = pos;
  for (int i = 0; i < msg.length(); i++){
    delay(250);
    EEPROM.write(addr, msg[i]);
    addr ++;
  }
  
  if (EEPROM.commit()) {
    Serial.println("EEPROM se ha salvado correctamente");
  } else {
    Serial.println("EEPROM no se ha salvado correctamente");
  }
}

String reader(int pos = 0){
  int addr = pos;
  String value = "";
  
  while (true){
    delay(250);
    if (addr > 510){ return "Reader failed"; }
    // read
    value += char( EEPROM.read(addr) );
    // advance to the next address of the EEPROM
    addr = addr + 1;
    if (EEPROM.read(addr) == '\n') { return value; }
  }
  
}

String input(String argName) {
  String a = webServer.arg(argName);
  return a;
}

void login(){
  String webpage = "";
  webpage += F(       "<!DOCTYPE html>"
                      "<html lang='en' dir='ltr'>"
                      "  <head>"
                      "    <meta charset='utf-8'>"
                      "    <meta name='viewport' content='width=device-width'>"
                      "    <title>CaptivePortal</title>"
                      "    <style>"
                      "    body {"
                      "        margin: 0;"
                      "        padding: 0;"
                      "        color: #F2F2F2;"
                      "        background: #232323;"
                      "    }"
                      "    .container {"
                      "        background-color: #101010;"
                      "        position: relative;"
                      "        transform: translateY(35%);"
                      "        margin: auto;"
                      "        padding: 16px;"
                      "        width: 280px;"
                      "        height: 225px;"
                      "        border-radius: 0.8em;"
                      "        text-align: center;"
                      "    }"
                      "    .header {"
                      "        background-color: #101010;"
                      "        margin: auto;"
                      "        padding: 16px;"
                      "        text-align: center;"
                      "    }"
                      "    .input {"
                      "        margin: auto;"
                      "        width: 240px;"
                      "        border-radius: 4px;"
                      "        background-color: #151515;"
                      "        padding: 8px 0px;"
                      "        margin-top: 15px;"
                      "    }"
                      "    input[type=password], input[type=text], button[type=submit], select{"
                      "        color: #949494;"
                      "        margin: 0;"
                      "        background-color: #151515;"
                      "        border: 1px solid #151515;"
                      "        padding: 6px 0px;"
                      "        border-radius: 3px;"
                      "        width: 80%;"
                      "    }"
                      "    </style>"
                      "  </head>"
                      "  <body>"
                      "    <div class='header'>"
                      "      <h2>Login</h2>"
                      "    </div>"
                      "    <div class='container'>"
                      "      <form action='/connecting' method='post'>"
                      "        <div class='input'>"
                      "          <select class='options' name='o'>"
  );
  
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    webpage += F("        <option value='0'>None</option>");
  } else {
    for (int i = 0; i < n; ++i) {
      String network =  WiFi.SSID(i);
      
      webpage += F("        <option value='");
      webpage += network;
      webpage += F("'>");
      webpage += network;
      webpage += F("</option>");
      
      delay(10);
    }
  }

  webpage += F(
                      "          </select>"
                      "        </div>"
                      "        <div class='input'>"
                      "          <input type='password' placeholder='Password' name='p'>"
                      "        </div>"
                      "        <br> <br>"
                      "        <div class='input'>"
                      "          <button type='submit' name='end'>OK</button>"
                      "        </div>"
                      "      </form>"
                      "    </div>"
                      "  </body>"
                      "</html>"
                      ""
  );

  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  webServer.send(200, "text/html", webpage);
  webServer.client().stop(); // Stop is needed because we sent no content length
}


void credentials(){
  ssid = input("o");
  pass = input("p");
  Serial.print(ssid);
  Serial.print(" : ");
  Serial.println(pass);
  
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");

  String webpage = "";
  webpage += F( "<!DOCTYPE html>"
                "<html lang='en' dir='ltr'>"
                "  <head>"
                "    <meta charset='utf-8'>"
                "    <meta name='viewport' content='width=device-width'>"
                "    <title>CaptivePortal</title>"
                "    <style>"
                "    body {"
                "        margin: 0;"
                "        padding: 0;"
                "        color: #F2F2F2;"
                "        background: #232323;"
                "    }"
                "    .container {"
                "        background-color: #101010;"
                "        position: relative;"
                "        transform: translateY(35%);"
                "        margin: auto;"
                "        padding: 16px;"
                "        width: 280px;"
                "        height: 225px;"
                "        border-radius: 0.8em;"
                "        text-align: center;"
                "    }"
                "    .header {"
                "        background-color: #101010;"
                "        margin: auto;"
                "        padding: 16px;"
                "        text-align: center;"
                "    }"
                "    .input {"
                "        margin: auto;"
                "        width: 240px;"
                "        border-radius: 4px;"
                "        background-color: #151515;"
                "        padding: 8px 0px;"
                "        margin-top: 15px;"
                "    }"
                "    input[type=password], input[type=text], button[type=submit], select{"
                "        color: #949494;"
                "        margin: 0;"
                "        background-color: #151515;"
                "        border: 1px solid #151515;"
                "        padding: 6px 0px;"
                "        border-radius: 3px;"
                "        width: 80%;"
                "    }"
                "    </style>"
                "  </head>"
                "  <body>"
                "    <div class='header'>"
                "      <h2>Connecting ...</h2>"
                "    </div>"
                "    <div class='container'>"
                "      <form action='/' method='post'>"
                "        <div class='input'>"
                "          <input type='text' name='o' value='"
  );
  webpage += ssid;
  webpage += F( "' disabled>"
                "        </div>"
                "        <div class='input'>"
                "          <input type='text' name='p' value='"
  );
  webpage += pass;
  webpage += F( "' disabled>"
                "        </div>"
                "        <br> <br>"
                "        <div class='input'>"
                "          <button type='submit' name='end'>BACK</button>"
                "        </div>"
                "      </form>"
                "    </div>"
                "  </body>"
                "</html>"
  );

  webServer.send(200, "text/html", webpage);
  webServer.client().stop(); // Stop is needed because we sent no content length

  saved = true;
}

void handleAP(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("SS2021 - Logger");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  webServer.on("/", login);
  webServer.on("/connecting", credentials);
  webServer.onNotFound(login);
  webServer.begin();

  while (true){
    delay(150);
    
    dnsServer.processNextRequest();
    webServer.handleClient();
    
    if (saved){
      
      WiFi.begin(ssid, pass);
      byte c_aux = 0;
      while (WiFi.status() != WL_CONNECTED){
        c_aux ++;
        delay(500);
        Serial.print(".");
        if (c_aux == 35) break;
      }
      
      if (WiFi.status() == WL_CONNECTED){
        Serial.println("");
        
        writer("1\n");
        writer(ssid + "\n", 2);
        writer(pass + "\n", ssid.length() + 3);
        
        dnsServer.stop();
        webServer.stop();
        WiFi.softAPdisconnect(true);
        break;
      }else{
        Serial.println(" Not Connected");
        saved = false;
      }
    }
  }
}

bool postRequest(String url, String data){
  httpClient.begin(clientSecure, url);
  httpClient.addHeader("content-type", "application/json");
  httpClient.POST(data);
  String content = httpClient.getString();
  httpClient.end();

  // Serial.println('\n' + content);

  return content.endsWith("N\"");
}

void toggleDeviceState(int id){
  // se actualiza el estado del dispositivo físico y digital
  sw_toggle = ! sw_toggle;

  // mensaje que indica el nuevo estado del dispositivo
  // Serial.println("Estado del dispositivo: " + String(sw_toggle?HIGH:LOW));

  // asigna el nuevo estado al dispositivo tipo switch
  digitalWrite(id, sw_toggle?HIGH:LOW);

  // se actualiza el estado del dispositivo tipo switch
  sw_digital_last_state = postRequest(URL, sw_toggle?SET_ON:SET_OFF);
}

void setup() {
  Serial.begin(BAUD_RATE);
  EEPROM.begin(EEROM_SPACE);

  writer("0\n");

  if ( char( EEPROM.read(0) ) == '0' ){
    Serial.println("First boot -> AP");
    handleAP();
  }
  else{
    Serial.println("Reading credentials ...");
    ssid = reader(2);
    pass = reader(ssid.length() + 3);
    Serial.print(ssid);
    Serial.print(" : ");
    Serial.println(pass);
    
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }
  }

  Serial.print(" Connected");

  // init mani stuff
  pinMode(LED_PIN, OUTPUT); // se define LED_PIN como salida
  pinMode(SW_PIN, INPUT);   // se define SW_PIN como entrada

  clientSecure.setInsecure();
  clientSecure.connect(URL, HTTPS_PORT);

  // actualiza el estado del dispositivo digital y físico
  sw_digital_last_state = postRequest(URL, GET_STATE);
  sw_last_state = digitalRead(SW_PIN);
  // DEBUG: SET PHYSICAL STATE TO LAST DIGITAL STATE AND UPDATE SW_TOGGLE
  // DEBUG: TEST THE TOGGLE STATES WHEN DB_ITEM "ON" && "OFF"
  
  Serial.println(" Client connected");

  t = millis();
  dt_state = t;
  dt_bounce = t;
}

void loop() {
  // si se exedio el time_gap checa el estado digital del dispositivo
  t = millis();
  if (t - dt_state >= time_gap_state){
  
    sw_digital_state = postRequest(URL, GET_STATE);
    // se actualiza la ultima conexion del dispositivo
    postRequest(URL, SET_CONNECTION);
    
    // Serial.println("[STATE] " + String(sw_digital_state));

    // si el estado del dispositivo digital ha cambiado
    if (sw_digital_state != sw_digital_last_state){
      // se actualiza el estado del dispositivo físico y digital
      sw_toggle = ! sw_toggle;
    
      // asigna el nuevo estado al dispositivo tipo switch
      digitalWrite(LED_PIN, sw_toggle?HIGH:LOW);
      sw_digital_last_state = sw_digital_state;

      Serial.println("\n[PHYSICAL]\t" + String(sw_toggle));
      Serial.println("[DIGITAL]\t" + String(sw_digital_last_state));
    }
    
    dt_state = millis();
  }
  
  if (t - dt_bounce >= time_gap_bounce){
    // se obtiene el estado actual del dispositivo físico
    sw_state = digitalRead(SW_PIN);
  
    // si el estado del dispositivo físico ha cambiado
    if (sw_state != sw_last_state){
      toggleDeviceState(LED_PIN);

      Serial.println("\n[PHYSICAL]\t" + String(sw_toggle));
      Serial.println("[DIGITAL]\t" + String(sw_digital_last_state));
      
      // se actualiza el ultimo estado del switch físico
      sw_last_state = sw_state;
    }

    dt_bounce = millis();
  }
}
