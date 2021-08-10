#include <EEPROM.h>

String boot_state = "0\n";
String ssid = "nwtwork\n";
String pass = "password\n";
String s_ssid = "";
String s_pass = "";

void writter(String msg, int pos = 0){
  int addr = pos;
  for (int i = 0; i < msg.length(); i++){
    delay(250);
    EEPROM.write(addr, msg[i]);
    addr ++;
  }
  
  if (EEPROM.commit()) {
    Serial.println("EEPROM successfully committed");
  } else {
    Serial.println("ERROR! EEPROM commit failed");
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

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  writter(boot_state);
  writter(ssid, boot_state.length());
  writter(pass, boot_state.length() + ssid.length());

  if ( char( EEPROM.read(0) ) == '0' )
    Serial.println("First boot -> AP");
  else{
    Serial.println("Reading credentials ...");
    s_ssid = reader(boot_state.length());
    s_pass = reader(boot_state.length() + s_ssid.length() + 1);
    Serial.print(s_ssid);
    Serial.print(" : ");
    Serial.println(s_pass);
  }
}

void loop() {
  
}
