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
const String UUID =     "iot-skill-00";     // Identificador unico del dispositivo
#define TYPE            "SWITCH"            // Identificador para el tipo de dispositivo
#define AP_TITLE        "SS2021 - LOGGER"   // Identificador para el tipo de dispositivo
#define RESET_PIN       14                  // GPIO para controlar el reset
#define LED_PIN         4                   // GPIO para rele que controla el dispositivo
#define SW_PIN          16                  // GPIO para switch que controla el dispositivo
#define BAUD_RATE       115200              // Valor por defecto para NodeMCU
#define EEROM_SPACE     512                 // Espacio en EEROM para NodeMCU
#define HTTPS_PORT      443                 // HTTPS Port
#define ATTEMPS         30                  // Connection attemps -> 15 seg
#define URL             "https://km83gb3bwa.execute-api.us-east-1.amazonaws.com/default/iot-skill-api"
#define SET_UP_DEVICE   "{\"action\": \"setup-device\", \"device-id\": \"" + UUID + "\"}"
#define GET_STATE       "{\"action\": \"get-device-state\", \"device-id\": \"" + UUID + "\"}"
#define SET_CONNECTION  "{\"action\": \"set-device-connection\", \"device-id\": \"" + UUID + "\"}"
#define SET_ON          "{\"action\": \"set-device-state\", \"device-id\": \"" + UUID + "\", \"value\": \"ON\"}"
#define SET_OFF         "{\"action\": \"set-device-state\", \"device-id\": \"" + UUID + "\", \"value\": \"OFF\"}"
const String STYLE =    "    <style>"
                        "    html{"
                        "        display: table;"
                        "        position: absolute;"
                        "        top: 0;"
                        "        left: 0;"
                        "        height: 100%;"
                        "        width: 100%;"
                        "    }"
                        "    body {"
                        "        display: table-cell;"
                        "        vertical-align: middle;"
                        "        margin: 0;"
                        "        padding: 0;"
                        "        color: #F2F2F2;"
                        "        background: #232323;"
                        "    }"
                        "    .container {"
                        "        background-color: #101010;"
                        "        margin: auto;"
                        "        padding: 16px;"
                        "        width: 280px;"
                        "        border-radius: 0.8em;"
                        "        text-align: center;"
                        "    }"
                        "    .input {"
                        "        margin: auto;"
                        "        width: 240px;"
                        "        border-radius: 4px;"
                        "        background-color: #151515;"
                        "        margin-top: 15px;"
                        "        margin-bottom: 15px;"
                        "    }"
                        "    input[type=password], input[type=text], button[type=submit], select{"
                        "        color: #949494;"
                        "        margin: 0;"
                        "        background-color: #151515;"
                        "        border: 1px solid #151515;"
                        "        padding: 8px 0px;"
                        "        border-radius: 3px;"
                        "        width: 80%;"
                        "    }"
                        "    button[type=submit]{"
                        "        width: 100%;"
                        "    }"
                        "    button[type=submit]:active {"
                        "        background-color: gray;"
                        "    }"
                        "    </style>";

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
    
    // read
    value += char( EEPROM.read(addr) );
    // advance to the next address of the EEPROM
    addr = addr + 1;

    if (addr >= EEROM_SPACE - 1)
      return "Reader failed";
    else if (EEPROM.read(addr) == '\n')
      return value;
  }
  
}

void cleaner(){
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < EEROM_SPACE; i++) {
    delay(10);
    EEPROM.write(i, 0);
  }
  if (EEPROM.commit()) {
    Serial.println("EEPROM se ha eliminado correctamente");
  } else {
    Serial.println("EEPROM no se ha eliminado correctamente");
  }
}

String input(String argName) {
  String a = webServer.arg(argName);
  return a;
}

void login(){
  String webpage = "";
  webpage += F( "<!DOCTYPE html>"
                "<html lang='en' dir='ltr'>"
                "  <head>"
                "    <meta charset='utf-8'>"
                "    <meta name='viewport' content='width=device-width'>"
                "    <title>CaptivePortal</title>");
  webpage += STYLE;
  webpage += F( "  </head>"
                "  <body>"
                "    <div class='container'>"
                "      <h3>Login</h3> <br>"
                "      <form action='/connecting' method='post'>"
                "        <div class='input'>"
                "          <select class='options' name='o'>");
  
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

  webpage += F( "          </select>"
                "        </div>"
                "        <div class='input'>"
                "          <input type='password' placeholder='Password' name='p'>"
                "        </div>"
                "        <br>"
                "        <div class='input'>"
                "          <button type='submit' name='end'>OK</button>"
                "        </div>"
                "      </form>"
                "    </div>"
                "  </body>"
                "</html>");

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
                "    <title>CaptivePortal</title>");
  webpage += STYLE;
  webpage += F( "    <script type='text/javascript'>"
                "        var seconds = 30;"
                "        var countdown = setInterval(function() {"
                "          seconds--;"
                "          if (seconds <= 0) {"
                "            document.getElementById('header').textContent = 'Error, favor de ingresar las credenciales de nuevo';"
                "            document.getElementById('button').textContent = 'ATRAS';"
                "            document.getElementById('button').disabled = false;"
                "            clearInterval(countdown);"
                "          }else {"
                "            document.getElementById('countdown').textContent = seconds;"
                "          };"
                "        }, 1000);"
                "    </script>"
                "  </head>"
                "  <body>"
                "    <div class='container'>"
                "      <h4 id='header'>Configurando dispositivo <span id='countdown'>30</span></h4> <br>"
                "      <form action='/' method='post'>"
                "        <div class='input'>"
                "          <input type='text' name='o' value='");
  webpage += ssid;
  webpage += F( "' disabled>"
                "        </div>"
                "        <div class='input'>"
                "          <input type='text' name='p' value='");
  webpage += pass;
  webpage += F( "' disabled>"
                "        </div>"
                "        <div class='input'>"
                "          <input type='text' name='uuid' value='");
  webpage += UUID;
  webpage += F( "' disabled>"
                "        </div>"
                "        <div class='input'>"
                "          <input type='text' name='type' value='");
  webpage += TYPE;
  webpage += F( "' disabled>"
                "        </div>"
                "        <br>"
                "        <div class='input'>"
                "          <button id='button' type='submit' name='end' disabled>...</button>"
                "        </div>"
                "      </form>"
                "    </div>"
                "  </body>"
                "</html>");

  webServer.send(200, "text/html", webpage);
  webServer.client().stop(); // Stop is needed because we sent no content length

  saved = true;
}

void connectToWiFi(String wifi_name, String wifi_pass){
  WiFi.begin(wifi_name, wifi_pass);
  byte c_aux = 0;
  while (WiFi.status() != WL_CONNECTED){
    c_aux ++;
    delay(500);
    Serial.print(".");
    if (c_aux == ATTEMPS) break;
  }
}

void handleAP(){
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_TITLE);

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
      
      connectToWiFi(ssid, pass);
      
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

  if (content.length() == 0)
    return sw_digital_last_state;
  else
    return content.endsWith("N\"");
}

void toggleDeviceState(int id){
  // se actualiza el estado del dispositivo físico y digital
  sw_toggle = ! sw_toggle;

  // asigna el nuevo estado al dispositivo tipo switch
  digitalWrite(id, sw_toggle?HIGH:LOW);

  // se actualiza el estado del dispositivo tipo switch
  if (WiFi.status() == WL_CONNECTED)
    sw_digital_last_state = postRequest(URL, sw_toggle?SET_ON:SET_OFF);
  else
    sw_digital_last_state = sw_toggle?true:false;
}

void setup() {
  Serial.begin(BAUD_RATE);
  EEPROM.begin(EEROM_SPACE);

  pinMode(LED_PIN, OUTPUT);           // se define LED_PIN como salida
  pinMode(SW_PIN, INPUT);             // se define SW_PIN como entrada
  pinMode(RESET_PIN, INPUT);          // se define RESET_PIN como entrada
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  char first = char( EEPROM.read(0) );
  //writer("0\n");
  
  if ( first == '1' ){
    Serial.println("Leyendo credenciales ...");
    ssid = reader(2);
    pass = reader(ssid.length() + 3);
    Serial.print(ssid);
    Serial.print(" : ");
    Serial.println(pass);
    
    connectToWiFi(ssid, pass);
  } else {
    Serial.println("Primer boot -> AP");
    handleAP();
  }

  Serial.print(" Conectado");

  // API setup
  clientSecure.setInsecure();
  clientSecure.connect(URL, HTTPS_PORT);

  // actualiza el estado del dispositivo digital y físico
  sw_digital_last_state = postRequest(URL, SET_UP_DEVICE);
  sw_last_state = digitalRead(SW_PIN);
  // DEBUG: SET PHYSICAL STATE TO LAST DIGITAL STATE AND UPDATE SW_TOGGLE
  // DEBUG: TEST THE TOGGLE STATES WHEN DB_ITEM "ON" && "OFF"
  
  Serial.println(" Cliente conectado");
  digitalWrite(LED_BUILTIN, LOW);

  t = millis();
  dt_state = t;
  dt_bounce = t;
}

void loop() {
  // DEBUG: BLACKOUT -> KEEP STATES UNCHANGED
  
  // si se exedio el time_gap checa el estado digital del dispositivo
  t = millis();
  if ((t - dt_state >= time_gap_state) && (WiFi.status() == WL_CONNECTED)){
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

      Serial.println("\n[FISICO]\t" + String(sw_toggle));
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

      Serial.println("\n[FISICO]\t" + String(sw_toggle));
      Serial.println("[DIGITAL]\t" + String(sw_digital_last_state));
      
      // se actualiza el ultimo estado del switch físico
      sw_last_state = sw_state;
    }

    dt_bounce = millis();
  }

  if (digitalRead(RESET_PIN)){
    delay(500);
    Serial.println("RESET PRESIONADO");
    cleaner();
    ESP.reset();
  }
}
