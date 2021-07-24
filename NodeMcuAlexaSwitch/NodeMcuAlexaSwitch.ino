/* 
 * Descripcíon del programa
 * - Control ON/OFF de un dispositivo de potencia:
 *    [+] Por voz               (Echo Dot)
 *    [+] Por aplicación móvil  (Alexa App)
 *    [+] Por Switch            (Manual)
 *    
 * Librerias:
 * - La libreria SinricPro facilita la comunicacion con el API de la Skill
 *   que es de donde recibimos la señal para encender o apagar el rele
 *   
 * Documentación oficial para la libreria SinricPro
 * - https://sinricpro.github.io/esp8266-esp32-sdk
 */

/* Librerias */
#include <ESP8266WiFi.h>      // Conexión a internet
#include <SinricPro.h>        // Comunicacion con el API
#include <SinricProSwitch.h>  // Dispositivo tipo Switch

/* Constantes */
#define WIFI_SSID         "INFINITUM2372_2.4"    
#define WIFI_PASS         "uH0L0Ybioa"
#define APP_KEY           "0ebe9f16-2810-4aec-ac14-b45947a3bd53"
#define APP_SECRET        "39b88368-2ed9-4d7c-8836-89593c493af0-d45ba$
#define SWITCH_ID         "60456f2e1516f07c9023c6af"

#define LED_PIN   4           // GPIO para rele que controla al FOCO
#define SW_PIN   16           // GPIO para switch que controla al FOCO
#define BAUD_RATE     115200  // Valor por defecto para NodeMCU

/* Variables */
byte sw_state = 0;            // Conserva el estado actual del switch físico
byte sw_last_state = 0;       // Conserva el ultimo estado del switch físico
bool sw_toggle = false;       // Conserva el estado del switch digital y físico

/* bool onPowerState(String deviceId, bool &state) 
 *
 * Se ejecuta al hacer un request desde el Echo Dot o la aplicación móvil
 * y recibe los siguientes parametros:
 * 
 * - String deviceId
 *    contiene el id del dispositivo tipo switch
 * - bool state
 *    contiene el nuevo estado del dispositivo tipo switch (on | off)
 * 
 * return
 *  devuelve true si el request fue exitoso, de otra manera false
 */
bool onPowerState(const String &deviceId, bool &state) {
  // mensaje que indica el nuevo estado del dispositivo
  Serial.printf("Dispositivo %s esta %s (via SinricPro) \r\n", deviceId.c_str(), state?"encendido":"apagado");

  // asigna el nuevo estado al dispositivo tipo switch
  digitalWrite(LED_PIN, state?HIGH:LOW);

  // se actualiza el estado del switch físico
  sw_toggle = ! sw_toggle;

  return true; // request fue exitoso
}

/* Funcion que establece conexion a internet */
void setupWiFi() {
  // establece conexión con el modem
  Serial.printf("\r\n[Wifi]: Estableciendo conexion");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // el bucle espera hasta hallar conexión a internet
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  
  // mensaje para confirmar conexión
  Serial.printf("conectado.\r\n[WiFi]: IP %s\r\n", WiFi.localIP().toString().c_str());
}
/* Prepara a SinricPro con el dispositivo a controlar */
// Habilita el dispositivo tipo switch en SinricPro
SinricProSwitch& mySwitch = SinricPro[SWITCH_ID];
void setupSinricPro() {
  // asigna la funcion que maneja el estado del dispositivo tipo switch
  mySwitch.onPowerState(onPowerState);

  // nos conectamos con el API de SinricPro
  SinricPro.onConnected([](){ Serial.printf("Conectado a SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Desconectado de SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

/* Preparamos los pines y las conexiones necesarias */
void setup() {
  pinMode(LED_PIN, OUTPUT); // se define LED_PIN como salida
  pinMode(SW_PIN, INPUT);   // se define SW_PIN como entrada

  
  Serial.begin(BAUD_RATE);  // se define el Bauld Rate para NodeMcu
  Serial.printf("\r\n\r\n");

  // se establece conexion a internet y posteriormente con el API
  setupWiFi();
  setupSinricPro();
}
  
/* Loop principal */
void loop() {
  // se encarga de manejar los comandos de voz y solicitudes desde la app
  SinricPro.handle();

  // se obtiene el estado actual del switch físico
  sw_state = digitalRead(SW_PIN);

  // si el estado del switch físico ha cambiado
  if (sw_state != sw_last_state){
    // se actualiza el estado del switch físico
    sw_toggle = ! sw_toggle;
    
    // mensaje que indica el nuevo estado del foco
    Serial.println("Estado del foco: " + String(sw_toggle?HIGH:LOW));
    
    // asigna el nuevo estado al dispositivo tipo switch
    digitalWrite(LED_PIN, sw_toggle?HIGH:LOW);

    // se actualiza el estado del dispositivo tipo switch
    mySwitch.sendPowerStateEvent(sw_toggle);

    // se actualiza el ultimo estado del switch físico
    sw_last_state = sw_state;
  }
}
