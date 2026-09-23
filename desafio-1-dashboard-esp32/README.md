# Desafio 15-A — Dashboard IoT com ESP32

Este projeto publica a temperatura do DHT11 a cada 15 segundos no Adafruit IO e
recebe o comando do dashboard para mover um microservo SG90.

## Hardware

- Placa: **WIFIESP-WROOM-32 DEVKITV1**.
- DHT11: VCC em 3,3 V, GND em GND e DATA na GPIO 4.
- Servo SG90: sinal na GPIO 18, VCC em 5 V e GND comum ao ESP32.

### Ligações na protoboard

Com o conector USB do ESP32 apontado para baixo, use os pinos impressos na
placa. Não use o Arduino Uno neste desafio.

| Componente | Pino do componente | Conectar ao ESP32 |
| --- | --- | --- |
| DHT11 | Pino 1 — VCC | `3V3` |
| DHT11 | Pino 2 — DATA | `D4` / GPIO 4 |
| DHT11 | Pino 3 — NC | Não conectar |
| DHT11 | Pino 4 — GND | `GND` |
| Servo SG90 | Laranja/amarelo — sinal | `D18` / GPIO 18 |
| Servo SG90 | Vermelho — positivo | `VIN` / 5 V ou fonte externa de 5 V |
| Servo SG90 | Marrom/preto — GND | `GND` |

No DHT11 sem placa adaptadora, adicione um resistor de **10 kΩ** entre os pinos
1 (VCC) e 2 (DATA). Com a grade azul voltada para você e os pinos para baixo,
a ordem é VCC, DATA, NC e GND, da esquerda para a direita.

> Se o servo for alimentado por uma fonte externa de 5 V, una o GND dessa fonte
ao GND do ESP32. Não alimente o servo pelo pino de 3,3 V.

Para o primeiro teste, valide Wi-Fi e DHT11 antes de ligar o servo. O servo pode
causar reinicialização se a alimentação USB não fornecer corrente suficiente;
nesse caso, use uma fonte externa estabilizada de 5 V, mantendo o GND comum.

## Configuração do Adafruit IO

1. Crie os feeds `telemetria` e `atuador`.
2. No dashboard, adicione um gráfico de linha para `telemetria`.
3. Adicione um botão Toggle para `atuador`, enviando `1` para ligar e `0` para desligar.
4. Copie `include/secrets.h.example` para `include/secrets.h` e informe o Wi-Fi,
   usuário e chave do Adafruit IO. O arquivo com as credenciais não é enviado ao Git.

Para uma rede Wi-Fi aberta, use uma senha vazia:

```cpp
#define WIFI_SSID "CATOLICASC"
#define WIFI_PASSWORD ""
```

O feed `telemetria` recebe a temperatura em graus Celsius, em formato numérico,
para que o gráfico de linha seja exibido corretamente.

## Upload e monitor serial

A porta configurada é `COM3` e o monitor opera em `115200` baud.

```bash
pio run -t upload
pio device monitor
```

## Evidências esperadas

- Monitor serial mostrando conexão e reconexão ao Wi-Fi.
- Gráfico com leituras espaçadas em 15 segundos.
- Foto/vídeo do Toggle mudando a posição do servo.
