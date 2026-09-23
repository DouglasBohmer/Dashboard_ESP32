# Desafio 15-A — Dashboard IoT com ESP32

Este projeto publica a temperatura do DHT11 a cada 15 segundos no Adafruit IO e
recebe o comando do dashboard para mover um microservo SG90.

## Hardware

- Placa: **WIFIESP-WROOM-32 DEVKITV1**.
- DHT11: VCC em 3,3 V, GND em GND e DATA na GPIO 4.
- Servo SG90: sinal na GPIO 18, VCC em 5 V e GND comum ao ESP32.

> Se o servo for alimentado por uma fonte externa de 5 V, una o GND dessa fonte
ao GND do ESP32. Não alimente o servo pelo pino de 3,3 V.

## Configuração do Adafruit IO

1. Crie os feeds `telemetria` e `atuador`.
2. No dashboard, adicione um gráfico de linha para `telemetria`.
3. Adicione um botão Toggle para `atuador`, enviando `1` para ligar e `0` para desligar.
4. Copie `include/secrets.h.example` para `include/secrets.h` e informe o Wi-Fi,
   usuário e chave do Adafruit IO. O arquivo com as credenciais não é enviado ao Git.

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
