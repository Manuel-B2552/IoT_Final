#include <DHT.h>

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
#define DHTTYPE DHT11

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

// Definimos los pines de los LEDs
#define LED_BLUE 2
#define LED_GREEN -3
#define LED_RED 4

void setup() {
  // Inicializamos comunicación serie
  Serial.begin(9600);

  // Comenzamos el sensor DHT
  dht.begin();

  // Configuramos los pines de los LEDs como salidas
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Apagamos todos los LEDs al inicio
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
}

void loop() {
    // Esperamos 5 segundos entre medidas
  delay(5000);

  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahrenheit
  float f = dht.readTemperature(true);

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return;
  }

  // Calcular el índice de calor en Fahrenheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  // Lógica para encender los LEDs
  if (t > 30) { // Si la temperatura es mayor a 30 grados Celsius
    digitalWrite(LED_RED, HIGH); // Enciende el LED rojo
    digitalWrite(LED_GREEN, LOW); // Apaga el LED verde
    digitalWrite(LED_BLUE, LOW);  // Apaga el LED azul
  } else if (t > 20) { // Si la temperatura está entre 20 y 30 grados Celsius
    digitalWrite(LED_RED, LOW); // Apaga el LED rojo
    digitalWrite(LED_GREEN, HIGH); // Enciende el LED verde
    digitalWrite(LED_BLUE, LOW);  // Apaga el LED azul
  } else { // Si la temperatura es menor a 20 grados Celsius
    digitalWrite(LED_RED, LOW); // Apaga el LED rojo
    digitalWrite(LED_GREEN, LOW); // Apaga el LED verde
    digitalWrite(LED_BLUE, HIGH); // Enciende el LED azul
  }
}