#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>

// ---------- I2C ----------
#define I2CSlaveAdress1 0x14
#define I2CSlaveAdress2 0x09
#define I2C_SDA 21
#define I2C_SCL 22

uint8_t byteSlave1 = 0;
uint8_t byteSlave2 = 0;

uint8_t datoParaSlave1 = 0;
uint8_t datoParaSlave2 = 0;

// ---------- WiFi ----------
const char* ssid = "ParqueOk";
const char* password = "12345678";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);

// 0 = disponible, 1 = ocupado
uint8_t parqueos[8] = {0,0,0,0,0,0,0,0};

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_AP);
  WiFi.disconnect(true);
  delay(500);

  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  delay(500);

  Serial.println("Access Point iniciado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handle_OnConnect);
  server.on("/estado", handle_Estado);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  Serial.println("I2C iniciado");
}

void loop() {
  server.handleClient();

  static unsigned long lastI2C = 0;

  if (millis() - lastI2C >= 500) {
    lastI2C = millis();

    leerSlave1();
    enviarASlave2();

    leerSlave2();
    enviarASlave1();
  }

  delay(2);
}

void leerSlave1() {
  Wire.beginTransmission(I2CSlaveAdress1);
  Wire.write('S');
  uint8_t error1 = Wire.endTransmission(true);

  if (error1 == 0) {
    uint8_t bytesReceived = Wire.requestFrom(I2CSlaveAdress1, 1);

    if (bytesReceived > 0) {
      byteSlave1 = Wire.read();

      datoParaSlave2 = byteSlave1;

      for (int i = 0; i < 4; i++) {
        parqueos[i] = (byteSlave1 >> i) & 0x01;
      }

      Serial.print("Byte recibido de Slave 1: ");
      Serial.println(byteSlave1, BIN);
    } else {
      Serial.println("Slave 1 no envio datos");
    }
  } else {
    Serial.print("Error leyendo Slave 1: ");
    Serial.println(error1);
  }
}

void enviarASlave2() {
  Wire.beginTransmission(I2CSlaveAdress2);
  Wire.write(datoParaSlave2);
  uint8_t error2 = Wire.endTransmission(true);

  if (error2 == 0) {
    Serial.print("Dato enviado a Slave 2: ");
    Serial.println(datoParaSlave2, BIN);
  } else {
    Serial.print("Error enviando a Slave 2: ");
    Serial.println(error2);
  }
}

void leerSlave2() {
  Wire.beginTransmission(I2CSlaveAdress2);
  Wire.write('S');
  uint8_t error2 = Wire.endTransmission(true);

  if (error2 == 0) {
    uint8_t bytesReceived = Wire.requestFrom(I2CSlaveAdress2, 1);

    if (bytesReceived > 0) {
      byteSlave2 = Wire.read();

      datoParaSlave1 = byteSlave2;

      for (int i = 0; i < 4; i++) {
        parqueos[i + 4] = (byteSlave2 >> i) & 0x01;
      }

      Serial.print("Byte recibido de Slave 2: ");
      Serial.println(byteSlave2, BIN);
    } else {
      Serial.println("Slave 2 no envio datos");
    }
  } else {
    Serial.print("Error leyendo Slave 2: ");
    Serial.println(error2);
  }
}

void enviarASlave1() {
  Wire.beginTransmission(I2CSlaveAdress1);
  Wire.write(datoParaSlave1);
  uint8_t error1 = Wire.endTransmission(true);

  if (error1 == 0) {
    Serial.print("Dato enviado a Slave 1: ");
    Serial.println(datoParaSlave1, BIN);
  } else {
    Serial.print("Error enviando a Slave 1: ");
    Serial.println(error1);
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

void handle_Estado() {
  String json = "[";

  for (int i = 0; i < 8; i++) {
    json += String(parqueos[i]);

    if (i < 7) {
      json += ",";
    }
  }

  json += "]";

  server.send(200, "application/json", json);
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
  String ptr = "<!DOCTYPE html><html>\n";
  ptr += "<head>\n";
  ptr += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<meta http-equiv=\"refresh\" content=\"2\">\n";
  ptr += "<title>Parqueo Inteligente</title>\n";

  ptr += "<style>\n";
  ptr += "html{font-family:Arial;text-align:center;background:#101820;color:white;}\n";
  ptr += "body{margin:0;padding:20px;}\n";
  ptr += "h1{color:#00d4ff;margin-bottom:5px;}\n";
  ptr += "h3{color:#cccccc;margin-top:0;margin-bottom:25px;}\n";
  ptr += ".parking{display:grid;grid-template-columns:repeat(4,1fr);gap:20px;max-width:900px;margin:auto;}\n";
  ptr += ".slot{height:190px;border:4px solid #ffffff;border-radius:12px;position:relative;background:#2c3e50;box-shadow:0 0 15px rgba(0,0,0,0.4);}\n";
  ptr += ".slot-number{position:absolute;top:8px;left:10px;font-size:20px;font-weight:bold;}\n";
  ptr += ".light{width:28px;height:28px;border-radius:50%;position:absolute;top:10px;right:10px;box-shadow:0 0 15px currentColor;}\n";
  ptr += ".green{background:#00ff66;color:#00ff66;}\n";
  ptr += ".red{background:#ff3333;color:#ff3333;}\n";
  ptr += ".car{position:absolute;left:50%;top:55%;transform:translate(-50%,-50%);font-size:70px;}\n";
  ptr += ".status{position:absolute;bottom:10px;width:100%;font-size:18px;font-weight:bold;}\n";
  ptr += ".available{color:#00ff66;}\n";
  ptr += ".occupied{color:#ff3333;}\n";
  ptr += "</style>\n";

  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Parqueo Inteligente</h1>\n";
  ptr += "<h3>Estado de 8 espacios</h3>\n";
  ptr += "<div class=\"parking\">\n";

  for (int i = 0; i < 8; i++) {
    ptr += "<div class=\"slot\">\n";
    ptr += "<div class=\"slot-number\">P";
    ptr += String(i + 1);
    ptr += "</div>\n";

    if (parqueos[i]) {
      ptr += "<div class=\"light red\"></div>\n";
      ptr += "<div class=\"car\">&#128663;</div>\n";
      ptr += "<div class=\"status occupied\">Ocupado</div>\n";
    } else {
      ptr += "<div class=\"light green\"></div>\n";
      ptr += "<div class=\"status available\">Disponible</div>\n";
    }

    ptr += "</div>\n";
  }

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";

  return ptr;
}