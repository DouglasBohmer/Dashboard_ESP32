# Desafio 15-A — Dashboard IoT com ESP32

Este projeto publica a temperatura do DHT11 a cada 15 segundos no Adafruit IO e
recebe o comando do dashboard para mover um microservo SG90.

## Hardware

- Placa: **WIFIESP-WROOM-32 DEVKITV1**.
- DHT11: VCC em 3,3 V, GND em GND e DATA na GPIO 4.
- Servo SG90: sinal na GPIO 18, VCC em 5 V e GND comum ao ESP32.

### Alimentação pelo Arduino Uno e cabos macho-macho

O Arduino Uno pode ser usado **somente como fonte de 5 V**. Não envie código ao
Uno e não conecte pinos TX, RX ou digitais entre as duas placas.

1. Encaixe o ESP32 na protoboard, atravessando o vão central; os dois conjuntos
   de pinos devem ficar em blocos de contatos separados.
2. Encaixe o DHT11 na protoboard. Com a grade azul voltada para você e os pinos
   para baixo, a ordem é VCC, DATA, NC e GND, da esquerda para a direita.
3. Alimente o Uno pela USB conectada ao computador ou a um carregador USB.
4. Com um jumper macho-macho, ligue o pino `5V` do Uno ao trilho vermelho (+)
   da protoboard.
5. Com outro jumper macho-macho, ligue um `GND` do Uno ao trilho azul (-).
6. Ligue o trilho vermelho ao pino `VIN` do ESP32 e o trilho azul a um `GND` do
   ESP32, também usando jumpers macho-macho.

```text
USB do computador ──> Arduino Uno
                       5V  ──> trilho + ──> ESP32 VIN
                       GND ──> trilho - ──> ESP32 GND
```

Para fazer upload ou usar o monitor serial em `COM3`, **desconecte o fio entre
o 5V do Uno e o VIN do ESP32** e conecte somente a USB do próprio ESP32 ao
computador. Não una duas fontes de 5 V ao ESP32 ao mesmo tempo. Depois do
upload, retire a USB do ESP32 e restabeleça a alimentação pelo Uno para a
execução independente.

### Ligações na protoboard

Com o conector USB do ESP32 apontado para baixo, use os pinos impressos na
placa.

| Componente | Pino do componente | Conectar ao ESP32 |
| --- | --- | --- |
| DHT11 | Pino 1 — VCC | `3V3` |
| DHT11 | Pino 2 — DATA | `D4` / GPIO 4 |
| DHT11 | Pino 3 — NC | Não conectar |
| DHT11 | Pino 4 — GND | `GND` |
| Servo SG90 | Laranja/amarelo — sinal | `D18` / GPIO 18 |
| Servo SG90 | Vermelho — positivo | Fonte externa de 5 V |
| Servo SG90 | Marrom/preto — GND | `GND` |

No DHT11 sem placa adaptadora, adicione um resistor de **10 kΩ** entre os pinos
1 (VCC) e 2 (DATA). Com a grade azul voltada para você e os pinos para baixo,
a ordem é VCC, DATA, NC e GND, da esquerda para a direita.

Como todos os pinos do ESP32 e do DHT11 ficam na protoboard, os jumpers
macho-macho funcionam normalmente: uma ponta entra na fileira do componente e a
outra na fileira do destino. Para o servo, encaixe uma ponta macho de cada jumper
dentro de cada conector fêmea do servo e a outra ponta na protoboard:

```text
fileira do ESP32 D18 ── jumper macho-macho ── conector laranja/amarelo do servo
trilho GND           ── jumper macho-macho ── conector marrom/preto do servo
fonte 5 V externa    ── jumper macho-macho ── conector vermelho do servo
```

> Una o GND da fonte externa do servo ao GND do ESP32. Não alimente o servo
> pelo pino de 3,3 V.

Para o primeiro teste, valide Wi-Fi e DHT11 antes de ligar o servo. O servo pode
causar reinicialização se compartilhar o 5 V do Uno com o ESP32. Por isso, use
uma fonte externa estabilizada de 5 V para o servo, mantendo o GND comum.

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
