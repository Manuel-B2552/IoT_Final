#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Servo.h>
#include <DHT.h>

// Credenciales Wi-Fi
#ifndef STASSID
#define STASSID "iPhone de Nicolas" // iPhone de Nicolas - Barra
#define STAPSK "manuel123" // manuel123 - Medinabarra2001
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Configuración del servidor web
ESP8266WebServer server(80);

// Configuración del Servo
Servo switchServo;
int servoPosition = 0; // Posición inicial del servo (0 o 180 grados)

// Configuración de las farolas (LEDs)
const int FAROLA_1 = 4;  // GPIO 4 (D2)
const int FAROLA_2 = 5;  // GPIO 5 (D1)
const int FAROLA_3 = 12; // GPIO 12 (D6)
const int FAROLA_4 = 13; // GPIO 13 (D7) - Nueva farola
bool lucesEncendidas = false; // Estado inicial apagado
bool farola4Encendida = false; // Estado inicial de la cuarta farola apagada

// Configuración del sensor DHT
#define DHTPIN 14 // Pin GPIO donde está conectado el DHT
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
String logData = "";

// Función para manejar la raíz
void handleRoot() {
  digitalWrite(FAROLA_1, lucesEncendidas ? HIGH : LOW);
  digitalWrite(FAROLA_2, lucesEncendidas ? HIGH : LOW);
  digitalWrite(FAROLA_3, lucesEncendidas ? HIGH : LOW);
  digitalWrite(FAROLA_4, farola4Encendida ? HIGH : LOW);

  String html = "<!DOCTYPE html>\
<html lang='en'>\
<head>\
  <meta charset='UTF-8'>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <title>ESP8266 Control Panel</title>\
  <style>\
    body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background-color: #222; color: #fff; }\
    button { padding: 15px 30px; font-size: 16px; margin: 10px; cursor: pointer; }\
    #log { border: 1px solid #fff; padding: 15px; margin-top: 20px; background: #333; color: #0f0; height: 200px; overflow-y: scroll; }\
  </style>\
</head>\
<body>\
  <h1>Control de Servo y Farolas</h1>\
  <button onclick=\"toggleServo()\">Alternar Servo</button>\
  <button onclick=\"toggleLuces()\">" + String(lucesEncendidas ? "Apagar" : "Encender") + " Farolas</button>\
  <button onclick=\"toggleFarola4()\">" + String(farola4Encendida ? "Apagar" : "Encender") + " Farola 4</button>\
  <div id='log'>Cargando...</div>\
  <script>\
    function toggleServo() { fetch('/toggle-servo').catch(err => console.error(err)); }\
    function toggleLuces() { fetch('/toggle-luces').catch(err => console.error(err)); }\
    function toggleFarola4() { fetch('/toggle-farola4').catch(err => console.error(err)); }\
    function updateLog() {\
      fetch('/get-log')\
        .then(response => response.text())\
        .then(data => {\
          document.getElementById('log').innerHTML = data;\
        })\
        .catch(err => console.error(err));\
    }\
    setInterval(updateLog, 5000); // Actualizar el log cada 5 segundos\
  </script>\
</body>\
</html>";
  server.send(200, "text/html", html);
}

// Función para alternar el servo
void handleServoToggle() {
  servoPosition = servoPosition == 0 ? 170 : 0;
  switchServo.write(servoPosition);
  logData += "Servo toggled to " + String(servoPosition) + " degrees<br>";
  server.send(200, "text/plain", "Servo toggled");
}

// Función para alternar las luces
void handleLucesToggle() {
  lucesEncendidas = !lucesEncendidas;
  digitalWrite(FAROLA_1, lucesEncendidas ? HIGH : LOW);
  digitalWrite(FAROLA_2, lucesEncendidas ? HIGH : LOW);
  digitalWrite(FAROLA_3, lucesEncendidas ? HIGH : LOW);
  logData += "Luces " + String(lucesEncendidas ? "encendidas" : "apagadas") + "<br>";
  server.send(200, "text/plain", "Luces toggled");
}

// Función para alternar la cuarta farola
void handleFarola4Toggle() {
  farola4Encendida = !farola4Encendida;
  digitalWrite(FAROLA_4, farola4Encendida ? HIGH : LOW);
  logData += "Farola 4 " + String(farola4Encendida ? "encendida" : "apagada") + "<br>";
  server.send(200, "text/plain", "Farola 4 toggled");
}

// Función para obtener el log
void handleGetLog() {
  String coloredLog = "";
  int colorIndex = 0; // Índice para seleccionar el color

  // Lista de colores a utilizar
  const char* colors[] = {"#FF5733", "#33FF57", "#3357FF", "#F3F33A", "#FF33A1", "#33FFF6", "#F033FF", "#FF9F33"};

  // Separar el log en líneas
  int lineStart = 0;
  for (int i = 0; i < logData.length(); i++) {
    if (logData[i] == '<' && logData[i+1] == 'b' && logData[i+2] == 'r') {  // Detectar salto de línea
      String line = logData.substring(lineStart, i);  // Obtener una línea
      coloredLog += "<span style='color:" + String(colors[colorIndex % 8]) + ";'>" + line + "</span><br>"; // Aplicar color
      colorIndex++;  // Cambiar color
      lineStart = i + 4;  // Salto de línea
    }
  }

  // Agregar la última línea, si hay
  if (lineStart < logData.length()) {
    String line = logData.substring(lineStart);
    coloredLog += "<span style='color:" + String(colors[colorIndex % 8]) + ";'>" + line + "</span><br>";
  }

  server.send(200, "text/html", coloredLog);  // Enviar log con colores
}

void setup() {
  // Configuración de pines
  pinMode(FAROLA_1, OUTPUT);
  pinMode(FAROLA_2, OUTPUT);
  pinMode(FAROLA_3, OUTPUT);
  pinMode(FAROLA_4, OUTPUT);

  // Configuración inicial de las luces
  digitalWrite(FAROLA_1, LOW);
  digitalWrite(FAROLA_2, LOW);
  digitalWrite(FAROLA_3, LOW);
  digitalWrite(FAROLA_4, LOW);

  // Configuración del sensor DHT
  dht.begin();

  // Configuración del servo
  switchServo.attach(2); // GPIO donde está conectado el servo
  switchServo.write(servoPosition);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("Servicio MDNS iniciado");
  }

  // Configuración de rutas
  server.on("/", handleRoot);
  server.on("/toggle-servo", handleServoToggle);
  server.on("/toggle-luces", handleLucesToggle);
  server.on("/toggle-farola4", handleFarola4Toggle);
  server.on("/get-log", handleGetLog); // Nueva ruta para obtener el log
  server.onNotFound([]() { server.send(404, "text/plain", "Página no encontrada"); });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
  MDNS.update();

  // Actualizar datos del sensor DHT periódicamente
  static unsigned long lastDHTUpdate = 0;
  if (millis() - lastDHTUpdate >= 5000) { // Cada 5 segundos
    lastDHTUpdate = millis();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      logData += "Humedad: " + String(h) + " % - Temperatura: " + String(t) + " C<br>";
    } else {
      logData += "Error al leer el sensor DHT<br>";
    }
  }
}

