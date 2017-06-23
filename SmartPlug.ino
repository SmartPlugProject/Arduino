#include <SoftwareSerial.h>

#define DEBUG true

String server = "smart-plug-1.herokuapp.com";
String sensorId = "5949364c85abcf0004882693";

SoftwareSerial esp8266(2, 3);

void setup() {
  Serial.begin(19200);
  esp8266.begin(19200);
  
  Serial.println("ESP8266 Demo - Aguardando 1 segundo");
  Serial.println("--------------");
  delay(1000);
  Serial.println("Enviando RST");
  esp8266.println("AT+RST");
  receiveResponse();
  receiveResponse();
  Serial.println("Selecionando modo de operacao misto (AP e estacao)");
  esp8266.println("AT+CWMODE=3");
  receiveResponse();
  Serial.println("Listando redes visiveis");
  esp8266.println("AT+CWLAP");
  receiveResponse();
  Serial.println("Conectando a uma rede");
  String command = "AT+CWJAP=\"";
  command += "SSID";
  command += "\",\"";
  command += "19021992hora*";
  command += "\"";
  esp8266.println(command);
  receiveResponse();
  Serial.println("Listando endereco IP (pode dar ping no segundo deles)");
  esp8266.println("AT+CIFSR");
  receiveResponse();
}

void loop() {
  //getSensorList();
  postSensorData(2, 127);
}

String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while((time + timeout) > millis()) {
    while(esp8266.available()) {
      char c = esp8266.read();
      response += c;
    }
  }

  if (debug) {
    Serial.print(response);
  }

  return response;
}

void postSensorData(float current, float voltage) {
  esp8266.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");

  if (esp8266.find("OK")) {
    Serial.println("TCP conectado");
  } else {
    Serial.println("Erro ao conectar TCP");
  }

  delay(1000);

  String data = "voltage=" + (String)voltage + "&current=" + (String)current;

  String postRequest = 
    "PUT /sensor/saveData/" + sensorId + " HTTP/1.1\r\n" + 
    "Host: smart-plug-1.herokuapp.com\r\n" + 
    "Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data;
    //"Connection: close\r\n\r\n";

  String cmd = "AT+CIPSEND=";
  esp8266.print(cmd);
  esp8266.println(postRequest.length());
  delay(500);
  if(esp8266.find(">")) {
    Serial.println("Sending...");
  
    esp8266.print(postRequest);   
    if (esp8266.find("SEND OK")) {
      Serial.println("Packet sent");
    
      while(esp8266.available()) {
        String tmpResp = esp8266.readString();
        Serial.println(tmpResp);
      }
  
      esp8266.println("AT+CIPCLOSE");
    }
  }

}

void getSensorList() {
  esp8266.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  if (esp8266.find("OK")) {
    Serial.println("TCP conectado");
  } else {
    Serial.println("Erro ao conectar TCP");
  }

  delay(1000);


  String getRequest = "GET /sensor/list HTTP/1.1\r\nHost: smart-plug-1.herokuapp.com\r\nConnection: close\r\n\r\n";
  String cmd = "AT+CIPSEND=";
  esp8266.print(cmd);
  esp8266.println(getRequest.length());
  delay(500);
  if(esp8266.find(">")) {
    Serial.println("Sending...");
  
    esp8266.print(getRequest);   
    if (esp8266.find("SEND OK")) {
      Serial.println("Packet sent");
    
      while(esp8266.available()) {
        String tmpResp = esp8266.readString();
        Serial.println(tmpResp);
      }
  
      esp8266.println("AT+CIPCLOSE");
    }
  }

}

void receiveResponse() {
  int limit = 7000;
  unsigned long arrive = millis();
  boolean continuar = true;
  String s = "";
  unsigned long lastChar = 0;
  while (continuar) {
    if (esp8266.available()) {
      char c = esp8266.read();
      lastChar = millis();
      s += c;
      Serial.print(c);
      if (c == 10) {
        byte p = s.indexOf(13);
        String s1 = s.substring(0, p);
        if (s1 == "OK") continuar = false;
        if (s1 == "ready") continuar = false;
        if (s1 == "no change") continuar = false;
        if (s1 == "ERROR") continuar = false;
        s = "";
      }
    }
    if (millis() - arrive > limit) continuar = false;
  }
}

