
# Códigos fonte ESP8266 - Arduino IDE

Uso da EEPROM - Electrically-Erasable Programmable Read-Only Memory
------
* O ESP8266 **não tem EEPROM dedicada**, somente é possível utilizá-la através de uma emulação em um segmento da FLASH de até 4096KB. Cada Byte da EEPROM no ESP8266, tem um limite de 10.000 gravações e tantas leituras quanto necessário.

Software
* Função EEPROM::begin()
 * EEPROM.begin(4);

Necessária sempre que for ler ou escrever na EEPROM. O parâmetro é o tamanho de inicialização, que precisa estar entre 4 até 4096. Neste caso, foi iniciado o mínimo possível, 4 Bytes.

* Função EEPROM:commit()
 - EEPROM.commit();

Esta função salva o que escrevemos anteriormente, sem isto, não irá salvar! Também pode ser feito pelo comando EEPROM.end(), que também efetua o commit().


 
</center>

![Instalação PID](../../../Imagens/BibliotecaPID.png)

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266

```c++
/********************************************************
 * PID Exemplo Básico
 * Lê entrada analógica e controla uma saída PWM
 ********************************************************/

#include <PID_v1.h>

#define PIN_ENTRADA 0
#define PIN_SAIDA 3

//VAriáveis de configuração do PID
double setPoint, entrada, saida;

//Valores para os ganhos das constante do PID
double Kp=2, Ki=5, Kd=1;
PID controlePID(&entrada, &saida, &setPoint, Kp, Ki, Kd, DIRECT);

void setup() {
  entrada = analogRead(PIN_ENTRADA);
  setPoint = 100;
  controlePID.SetMode(AUTOMATIC);
}

void loop() {
  entrada = analogRead(PIN_ENTRADA);
  controlePID.Compute();
  analogWrite(PIN_SAIDA, saida);
}

```
