/* 
 * Descripcíon del programa
 * - Control por voz o aplicación móvil de un rele (Echo Dot y Alexa App)
 * - La libreria SinricPro facilita la comunicacion con el API de la Skill
 * - Que es de donde recibimos la señal para encender o apagar el rele
 * Documentación oficial para la libreria SinricPro
 * - https://sinricpro.github.io/esp8266-esp32-sdk
 */

/* Librerias */
#include <ESP8266WiFi.h>      // Conexión a internet
#include <SinricPro.h>        // Comunicacion con el API
#include <SinricProSwitch.h>  // Dispositivo tipo Switch

/* Constantes */
#define WIFI_SSID     ""
#define WIFI_PASS     ""
#define APP_KEY       ""
#define APP_SECRET    ""
#define SWITCH_ID     ""
#define BAUD_RATE     115200  // Valor por defecto para NodeMcu

#define LED_PIN   4   // GPIO para rele conectado al LED

/* bool onPowerState(String deviceId, bool &state) 
 *
 * Se ejecuta al hacer un request desde el Echo Dot o la aplicación móvil
 * y recibe los siguientes parametros
 *  String deviceId
 *    contiene el id del dispositivo tipo switch
 *  bool state
 *    contiene el nuevo estado del dispositivo tipo switch (on | off)
 * 
 * return
 *  devuelve true si el request fue exitoso, de otra manera false
 */
bool onPowerState(const String &deviceId, bool &state) {
  // mensaje que indica el nuevo estado del dispositivo
  Serial.printf("Dispositivo %s esta %s (via SinricPro) \r\n", deviceId.c_str(), state?"encendido":"apagado");
  // asigna el nuevo estado al dispositivo
  digitalWrite(LED_PIN, state?HIGH:LOW);
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
  Serial.printf("conectadi.\r\n[WiFi]: IP %s\r\n", WiFi.localIP().toString().c_str());
}

/* Prepara a SinricPro con el dispositivo a controlar */
void setupSinricPro() {
  // habilita el dispositivo a SinricPro
  SinricProSwitch& mySwitch = SinricPro[SWITCH_ID];

  // asigna la funcion que maneja al dispositivo
  mySwitch.onPowerState(onPowerState);

  // nos conectamos con el API de SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}

/* Preparamos los pines y las conexiones necesarias */
void setup() {
  // se define LED_PIN como salida
  pinMode(LED_PIN, OUTPUT);

  // se define el Bauld Rate para NodeMcu
  Serial.begin(BAUD_RATE);
  Serial.printf("\r\n\r\n");

  // se establece conexion a internet y posteriormente con el API
  setupWiFi();
  setupSinricPro();
}

/* Loop principal */
void loop() {
  // se encarga de manejar los comandos de voz y solicitudes desde la app
  SinricPro.handle();
}
