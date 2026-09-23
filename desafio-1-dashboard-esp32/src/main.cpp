#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "secrets.h"

constexpr uint8_t PINO_DHT = 4;
constexpr uint8_t PINO_SERVO = 18;
constexpr uint8_t TIPO_DHT = DHT11;

constexpr uint16_t ANGULO_DESLIGADO = 0;
constexpr uint16_t ANGULO_LIGADO = 90;
constexpr unsigned long INTERVALO_TELEMETRIA_MS = 15000;
constexpr unsigned long INTERVALO_WIFI_MS = 10000;
constexpr unsigned long INTERVALO_MQTT_MS = 5000;

const char *BROKER_MQTT = "io.adafruit.com";
constexpr uint16_t PORTA_MQTT = 1883;

DHT dht(PINO_DHT, TIPO_DHT);
Servo servo;
WiFiClient clienteWiFi;
PubSubClient clienteMqtt(clienteWiFi);

char topicoTelemetria[96];
char topicoAtuador[96];
char idCliente[32];
unsigned long proximaTelemetria = 0;
unsigned long ultimaTentativaWifi = 0;
unsigned long ultimaTentativaMqtt = 0;

bool intervaloDecorrido(unsigned long agora, unsigned long ultimaTentativa,
                        unsigned long intervalo) {
  return agora - ultimaTentativa >= intervalo;
}

void definirAtuador(bool ligado) {
  servo.write(ligado ? ANGULO_LIGADO : ANGULO_DESLIGADO);
  Serial.printf("Atuador: %s (%u graus)\n", ligado ? "LIGADO" : "DESLIGADO",
                ligado ? ANGULO_LIGADO : ANGULO_DESLIGADO);
}

bool comandoLiga(const String &comando) {
  return comando == "1" || comando == "on" || comando == "liga" ||
         comando == "ligado" || comando == "true";
}

bool comandoDesliga(const String &comando) {
  return comando == "0" || comando == "off" || comando == "desliga" ||
         comando == "desligado" || comando == "false";
}

void aoReceberComando(char *topico, byte *payload, unsigned int tamanho) {
  String comando;
  comando.reserve(tamanho);

  for (unsigned int i = 0; i < tamanho; ++i) {
    comando += static_cast<char>(payload[i]);
  }

  comando.trim();
  comando.toLowerCase();
  Serial.printf("Mensagem recebida em %s: %s\n", topico, comando.c_str());

  if (comandoLiga(comando)) {
    definirAtuador(true);
  } else if (comandoDesliga(comando)) {
    definirAtuador(false);
  } else {
    Serial.println("Comando ignorado. Use 1/0, ON/OFF ou LIGA/DESLIGA.");
  }
}

void conectarWifi(unsigned long agora) {
  if (WiFi.status() == WL_CONNECTED ||
      !intervaloDecorrido(agora, ultimaTentativaWifi, INTERVALO_WIFI_MS)) {
    return;
  }

  ultimaTentativaWifi = agora;
  Serial.printf("Conectando ao Wi-Fi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void conectarMqtt(unsigned long agora) {
  if (WiFi.status() != WL_CONNECTED || clienteMqtt.connected() ||
      !intervaloDecorrido(agora, ultimaTentativaMqtt, INTERVALO_MQTT_MS)) {
    return;
  }

  ultimaTentativaMqtt = agora;
  Serial.print("Conectando ao MQTT... ");

  if (clienteMqtt.connect(idCliente, AIO_USERNAME, AIO_KEY)) {
    Serial.println("conectado.");
    clienteMqtt.subscribe(topicoAtuador);
    Serial.printf("Escutando: %s\n", topicoAtuador);
  } else {
    Serial.printf("falhou (codigo %d). Nova tentativa em 5 s.\n",
                  clienteMqtt.state());
  }
}

void publicarTelemetria() {
  const float temperatura = dht.readTemperature();
  const float umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Falha ao ler o DHT11. Verifique alimentacao e fio de dados.");
    return;
  }

  Serial.printf("Temperatura: %.1f C | Umidade: %.1f %%\n", temperatura, umidade);

  if (!clienteMqtt.connected()) {
    Serial.println("Telemetria nao enviada: MQTT ainda desconectado.");
    return;
  }

  char valor[12];
  snprintf(valor, sizeof(valor), "%.1f", temperatura);

  if (clienteMqtt.publish(topicoTelemetria, valor)) {
    Serial.printf("Telemetria enviada para %s: %s C\n", topicoTelemetria, valor);
  } else {
    Serial.println("Falha ao publicar a telemetria.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nDesafio 15-A: Dashboard IoT com ESP32");

  snprintf(topicoTelemetria, sizeof(topicoTelemetria), "%s/feeds/telemetria",
           AIO_USERNAME);
  snprintf(topicoAtuador, sizeof(topicoAtuador), "%s/feeds/atuador", AIO_USERNAME);
  snprintf(idCliente, sizeof(idCliente), "esp32-%08llX",
           static_cast<unsigned long long>(ESP.getEfuseMac()));

  dht.begin();
  servo.setPeriodHertz(50);
  servo.attach(PINO_SERVO, 500, 2400);
  definirAtuador(false);

  clienteMqtt.setServer(BROKER_MQTT, PORTA_MQTT);
  clienteMqtt.setCallback(aoReceberComando);

  const unsigned long agora = millis();
  proximaTelemetria = agora + INTERVALO_TELEMETRIA_MS;
  ultimaTentativaWifi = agora - INTERVALO_WIFI_MS;
  ultimaTentativaMqtt = agora - INTERVALO_MQTT_MS;
  conectarWifi(agora);
}

void loop() {
  const unsigned long agora = millis();

  conectarWifi(agora);
  conectarMqtt(agora);
  clienteMqtt.loop();

  if (static_cast<long>(agora - proximaTelemetria) >= 0) {
    publicarTelemetria();
    proximaTelemetria += INTERVALO_TELEMETRIA_MS;
  }
}
