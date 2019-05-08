# Códigos fonte ESP8266 - Arduino IDE

Acelerômetro utizando ESP8266
------

* Na utilização do DMP (Digital motion Processor)  com a IDE arduino e ESP8266 será necessário a importação da biblioteca **MPU6050**.
* Instalação da biblioteca **MPU6050**

![Instalação MPU6050](../../../Imagens/BibliotecaMPU6050.png)

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266 como receptor de códigos infravermelho

```c++

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

#define INTERRUPT_PIN 15 //D8

MPU6050 mpu;
```

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266 como emissor de códigos infravermelho

```c++

#include <IRremoteESP8266.h>//  Biblioteca para acesso ao sensor IR - ESP8266
#include <IRsend.h>         // Biblioteca auxiliar - deve ser incluida junto com a IRremoteESP8266
#include <IRutils.h>        // Biblioteca auxiliar - deve ser incluida junto com a IRremoteESP8266

// Configuracoes iniciais
#define pinoIR        2
#define POWER         0xE0E040BF // Ligar/Desligar 
#define SAMSUNG_BITS  32 // Tamanho do codigo de informacao para o dispositivo Sansung

IRsend irsend(pinoIR); //  Cria um emissor que codifica o sinal para controle de dispositivos

void setup() {
  Serial.begin(115200);
  irsend.begin(); //  Habilita o emissor para inicio de processamento dos codigos a serem enviados
}

void loop() {
  irsend.sendSAMSUNG(POWER, SAMSUNG_BITS); //  Codifica a informacao a ser enviada para o receptor
  delay(5000);
}

```
