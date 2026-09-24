#include <Arduino.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "secrets.h"

constexpr uint8_t PINO_DHT = 4;
constexpr uint8_t PINO_SERVO = 19;
constexpr uint8_t TIPO_DHT = DHT11;
constexpr uint8_t POSICAO_INICIAL = 0;
constexpr uint8_t POSICAO_ATIVA = 90;
constexpr uint16_t PORTA_MQTT = 1883;
constexpr unsigned long INTERVALO_ENVIO = 15000;
constexpr unsigned long INTERVALO_WIFI = 10000;
constexpr unsigned long INTERVALO_MQTT = 5000;

const char SERVIDOR_MQTT[] = "io.adafruit.com";

DHT sensor(PINO_DHT, TIPO_DHT);
Servo servo;
WiFiClient conexao;
PubSubClient mqtt(conexao);

char topicoTelemetria[96];
char topicoAtuador[96];
char idDispositivo[32];

unsigned long ultimoEnvio = 0;
unsigned long ultimaTentativaWifi = 0;
unsigned long ultimaTentativaMqtt = 0;
bool wifiEstavaConectado = false;

bool tempoAtingido(unsigned long agora, unsigned long anterior,
                   unsigned long intervalo) {
  return agora - anterior >= intervalo;
}

void moverServo(uint8_t angulo) {
  servo.write(angulo);
  Serial.printf("Servo em %u graus\n", angulo);
}

String montarMensagem(const byte *dados, unsigned int tamanho) {
  String mensagem;
  mensagem.reserve(tamanho);

  for (unsigned int indice = 0; indice < tamanho; ++indice) {
    mensagem += static_cast<char>(dados[indice]);
  }

  mensagem.trim();
  mensagem.toLowerCase();
  return mensagem;
}

void receberComando(char *topico, byte *dados, unsigned int tamanho) {
  const String mensagem = montarMensagem(dados, tamanho);
  Serial.printf("Comando em %s: %s\n", topico, mensagem.c_str());

  if (mensagem == "1" || mensagem == "90" || mensagem == "on" ||
      mensagem == "liga") {
    moverServo(POSICAO_ATIVA);
  } else if (mensagem == "0" || mensagem == "off" ||
             mensagem == "desliga") {
    moverServo(POSICAO_INICIAL);
  } else {
    Serial.println("Comando nao reconhecido");
  }
}

void atualizarWifi(unsigned long agora) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!wifiEstavaConectado) {
      wifiEstavaConectado = true;
      Serial.printf("Wi-Fi conectado. IP: %s\n",
                    WiFi.localIP().toString().c_str());
    }
    return;
  }

  wifiEstavaConectado = false;

  if (!tempoAtingido(agora, ultimaTentativaWifi, INTERVALO_WIFI)) {
    return;
  }

  ultimaTentativaWifi = agora;
  Serial.printf("Conectando ao Wi-Fi %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void atualizarMqtt(unsigned long agora) {
  if (WiFi.status() != WL_CONNECTED || mqtt.connected() ||
      !tempoAtingido(agora, ultimaTentativaMqtt, INTERVALO_MQTT)) {
    return;
  }

  ultimaTentativaMqtt = agora;

  if (!mqtt.connect(idDispositivo, AIO_USERNAME, AIO_KEY)) {
    Serial.printf("Falha MQTT: %d\n", mqtt.state());
    return;
  }

  mqtt.subscribe(topicoAtuador);
  Serial.println("Adafruit IO conectado");
}

void enviarLeitura(unsigned long agora) {
  if (!tempoAtingido(agora, ultimoEnvio, INTERVALO_ENVIO)) {
    return;
  }

  ultimoEnvio = agora;
  const float temperatura = sensor.readTemperature();

  if (isnan(temperatura)) {
    Serial.println("Falha na leitura do DHT11");
    return;
  }

  Serial.printf("Temperatura: %.1f C\n", temperatura);

  if (!mqtt.connected()) {
    Serial.println("MQTT desconectado");
    return;
  }

  char valor[12];
  snprintf(valor, sizeof(valor), "%.1f", temperatura);

  if (mqtt.publish(topicoTelemetria, valor)) {
    Serial.println("Telemetria enviada");
  } else {
    Serial.println("Falha no envio da telemetria");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PINO_DHT, INPUT_PULLUP);

  snprintf(topicoTelemetria, sizeof(topicoTelemetria), "%s/feeds/telemetria",
           AIO_USERNAME);
  snprintf(topicoAtuador, sizeof(topicoAtuador), "%s/feeds/atuador",
           AIO_USERNAME);
  snprintf(idDispositivo, sizeof(idDispositivo), "painel-%08llX",
           static_cast<unsigned long long>(ESP.getEfuseMac()));

  sensor.begin();
  ESP32PWM::allocateTimer(0);
  servo.setPeriodHertz(50);
  servo.attach(PINO_SERVO, 500, 2400);
  moverServo(POSICAO_INICIAL);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  mqtt.setServer(SERVIDOR_MQTT, PORTA_MQTT);
  mqtt.setCallback(receberComando);

  const unsigned long agora = millis();
  ultimaTentativaWifi = agora - INTERVALO_WIFI;
  ultimaTentativaMqtt = agora - INTERVALO_MQTT;
  atualizarWifi(agora);
}

void loop() {
  const unsigned long agora = millis();

  atualizarWifi(agora);
  atualizarMqtt(agora);

  if (mqtt.connected()) {
    mqtt.loop();
  }

  enviarLeitura(agora);
}
