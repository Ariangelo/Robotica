
# Códigos fonte ESP8266 - Arduino IDE

Uso da EEPROM - Electrically-Erasable Programmable Read-Only Memory
------
* O ESP8266 **não tem EEPROM dedicada**, somente é possível utilizá-la através de uma emulação em um segmento da FLASH de até 4096KB. Cada Byte da EEPROM no ESP8266, tem um limite de 10.000 gravações e tantas leituras quanto necessário.

Software
* Função EEPROM::begin()
  * EEPROM.begin(4);

Necessária sempre que for ler ou escrever na EEPROM. O parâmetro é o tamanho de inicialização, que precisa estar entre 4 até 4096. Neste caso, foi iniciado o mínimo possível, 4 Bytes.

* Função EEPROM::commit()
  * EEPROM.commit();

Esta função salva o que escrevemos anteriormente, sem isto, não irá salvar! Também pode ser feito pelo comando EEPROM.end(), que também efetua o commit().

* Para maiores informações consulte: https://www.arduino.cc/en/Reference/EEPROM

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266

```c++
/********************************************************
 * PID Exemplo Básico com uso da EEPROM
 ********************************************************/

#include "EEPROM.h"

//Ajustar os parametros para o projeto em desenvolvimento
struct Configuration {
  float fatorEsquerda;
  float fatorDireita;
  double Kp;
  double Ki;
  double Kd;
  double setpoint;
  int tempoAmostragem;
};

Configuration confValues;

void setup() {
  ... Códigos de inicialização
  
  EEPROM.begin(48);//Inicia a EEPROM com o tamanho da informação.
  getEEPROM();
  
  //setup PID
  setpoint = confValues.setpoint;
  objetivo = setpoint;
  pid.SetMode(AUTOMATIC);
  pid.SetTunings(confValues.Kp, confValues.Ki, confValues.Kd);
  pid.SetSampleTime(confValues.tempoAmostragem);
  pid.SetOutputLimits(-100, 100);
  
  ... Demais códigos
}

void loop() {
  ESPserialEvent();
  if (infoCompleta) {
    gravaEEPROM();
    pid.SetTunings(confValues.Kp, confValues.Ki, confValues.Kd);
    pid.SetSampleTime(confValues.tempoAmostragem);
    infoCompleta = false;
  }
  
  ... Demais códigos
}

void ESPserialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
      case 'a':
        confValues.fatorEsquerda = Serial.parseFloat();
        break;
      case 'b':
        confValues.fatorDireita = Serial.parseFloat();
        break;
      case 'p':
        confValues.Kp = Serial.parseFloat();
        break;
      case 'i':
        confValues.Ki = Serial.parseFloat();
        break;
      case 'd':
        confValues.Kd = Serial.parseFloat();
        break;
      case 's':
        setpoint = Serial.parseFloat();
        confValues.setpoint = setpoint;
        break;
      case 't':
        confValues.tempoAmostragem = Serial.parseInt();
        break;
    }
    infoCompleta = (c == fimInformacao) || (c == '\n');
  }
}

void getEEPROM() {
  EEPROM.get(0, confValues);
  if (confValues.tempoAmostragem == -1) {
    confValues.fatorEsquerda = 0.5;
    confValues.fatorDireita = 0.5;
    confValues.Kp = 70.0;
    confValues.Ki = 500.0;
    confValues.Kd = 2.0;
    confValues.setpoint = 103.0;
    confValues.tempoAmostragem = 5;
    gravaEEPROM();
  }
  mostrarEEPROM("Lendo EEPROM");
}

void gravaEEPROM() {
  EEPROM.put(0, confValues);
  EEPROM.commit();//Salva o dado na EEPROM.
  mostrarEEPROM("Gravando EEPROM");
}

void mostrarEEPROM (String titulo) {
  Serial.println(titulo);
  Serial.print("F. esquerdo= ");
  Serial.print(confValues.fatorEsquerda);
  Serial.print(" F. direito= ");
  Serial.print(confValues.fatorDireita);
  Serial.print(" (kP= ");
  Serial.print(confValues.Kp);
  Serial.print(", kI= ");
  Serial.print(confValues.Ki);
  Serial.print(", kD= ");
  Serial.print(confValues.Kd);
  Serial.print(", Set point= ");
  Serial.print(confValues.setpoint);
  Serial.print(", Tempo amostragem= ");
  Serial.println(confValues.tempoAmostragem);
}

```
