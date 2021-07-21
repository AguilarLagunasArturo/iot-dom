/* Librerias */
#include <ESP8266WiFi.h>      // Conexión a internet
#include <ESP8266HTTPClient.h>

/* Constantes */
#define WIFI_SSID   ""    
#define WIFI_PASS   ""
#define URL         "https://km83gb3bwa.execute-api.us-east-1.amazonaws.com/default/iot-skill-api"

#define LED_PIN     4         // GPIO para rele que controla al dispositivo
#define SW_PIN      16        // GPIO para switch que controla al dispositivo
#define BAUD_RATE   115200    // Valor por defecto para NodeMCU

/* Variables */
String toggle_state_data = "{\"action\": \"toggle-device-state\", \"device-id\": \"iot-skill-01\"}";
String get_state_data = "{\"action\": \"get-device-state\", \"device-id\": \"iot-skill-01\"}";
String set_on_state_data = "{\"action\": \"set-device-state\", \"device-id\": \"iot-skill-01\", \"value\": \"ON\"}";
String set_off_state_data = "{\"action\": \"set-device-state\", \"device-id\": \"iot-skill-01\", \"value\": \"OF\"}";

byte sw_state = 0;            // Conserva el estado actual del switch físico
byte sw_last_state = 0;       // Conserva el ultimo estado del switch físico
bool sw_toggle = false;       // Conserva el estado del switch digital y físico
bool sw_digital_state = false;
bool sw_digital_last_state = false;

unsigned long int t = 0;
unsigned long int dt = 0;
int time_gap = 5000;

WiFiClientSecure client;
HTTPClient httpClient;

/* Preparamos los pines y las conexiones necesarias */
void setup() {
  pinMode(LED_PIN, OUTPUT); // se define LED_PIN como salida
  pinMode(SW_PIN, INPUT);   // se define SW_PIN como entrada

  Serial.begin(BAUD_RATE);  // se define el Bauld Rate para NodeMcu
  Serial.printf("\r\n\r\n");

  // se establece conexion a internet
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.print(" Connected");

  client.setInsecure();
  client.connect(URL, 443);
  Serial.println(" Client connected");

  t = millis();
  dt = t;
}

/* Loop principal */
void loop() {
  // si se exedio el time_gap checa el estado digital del dispositivo
  t = millis();
  if (t - dt >= time_gap){
  
    sw_digital_state = postRequest(URL, get_state_data);
    Serial.println("[STATE] " + String(sw_digital_state));

    // si el estado del dispositivo digital ha cambiado
    if (sw_digital_state != sw_digital_last_state){
      toggleDeviceState(LED_PIN);
      sw_digital_last_state = sw_digital_state;
    }

    
    dt = millis();
  }

  // se obtiene el estado actual del dispositivo físico
  sw_state = digitalRead(SW_PIN);

  // si el estado del dispositivo físico ha cambiado
  if (sw_state != sw_last_state){
    toggleDeviceState(LED_PIN);
  }

}

bool postRequest(String url, String data){
  httpClient.begin(client, url);
  httpClient.addHeader("content-type", "application/json");
  httpClient.POST(data);
  String content = httpClient.getString();
  httpClient.end();

  Serial.println('\n' + content);

  return content.endsWith("N\"");
}

void toggleDeviceState(int id){
  // se actualiza el estado del dispositivo físico
  sw_toggle = ! sw_toggle;

  // mensaje que indica el nuevo estado del dispositivo
  Serial.println("Estado del dispositivo: " + String(sw_toggle?HIGH:LOW));

  // asigna el nuevo estado al dispositivo tipo switch
  digitalWrite(id, sw_toggle?HIGH:LOW);

  // se actualiza el estado del dispositivo tipo switch
  // mySwitch.sendPowerStateEvent(sw_toggle);

  // se actualiza el ultimo estado del switch físico
  sw_last_state = sw_state;
}
