# Códigos fonte ESP8266 - Arduino IDE

Infravermelho utizando ESP8266
------

:heavy_check_mark: [Link para: Esquema de ligação e simulação no Tinkercad](https://www.tinkercad.com/things/0rbIx4AF0Nc)
![Uso de sensor IR e controle remoto](https://csg.tinkercad.com/things/0rbIx4AF0Nc/t725.png?rev=1541075331212000000&s=d10c875abfeb46cf1f059a8de10aafd7&v=1&type=circuits)

* Na utilização do receptor infravermeloho (IR) com a IDE arduino e ESP8266 será necessário a importação da biblioteca **IRremoteESP8266**.
* Instalação da biblioteca **IRremoteESP8266**

![Instalação IRremoteESP8266](../../../Imagens/IR.png)

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266 como receptor de códigos infravermelho

```c++

#include <IRremoteESP8266.h>//  Biblioteca para acesso ao sensor IR - ESP8266
#include <IRrecv.h>         // Biblioteca auxiliar - deve ser incluida junto com a IRremoteESP8266
#include <IRutils.h>        // Biblioteca auxiliar - deve ser incluida junto com a IRremoteESP8266

// Configuracoes iniciais
#define pinoIR    2

IRrecv receptor(pinoIR);   //  Cria um receptor que decodifica o sinal do sensor IR - codigos do controle remoto
decode_results resultados; //  Variavel que aramzenara os resultados recebidos

void setup() {
  Serial.begin(115200);
  receptor.enableIRIn();  //  Habilita o receptor para inicio de processamento dos codigos recebidos do emissor IR
}

void loop() {
  if (receptor.decode(&resultados)) {   //  Decodifica o codigo da informacao enviada pelo emissor IR
    serialPrintUint64(resultados.value, HEX); //  Mostra o valor em HEX do resultado recebido
    Serial.println("");
    receptor.resume();  //  Continua a aguardar o envio de informacoes peso emissor IR
  }
}

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
