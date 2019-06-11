# Códigos fonte ESP8266 - Arduino IDE

MQTT utizando ESP8266
------
MQTT (Message Queuing Telemetry Transport) é um protocolo de mensagens dispositivos móveis de pequeno porte com otimização para redes TCP/IP. O esquema de troca de mensagens é baseado no modelo Publisher-Subscriber.

* [Informações adicionais sobre o protocolo MQTT](http://mqtt.org/)

![Esquema MQTT](https://www.survivingwithandroid.com/wp-content/uploads/2016/10/mqtt_publisher_subscriber-1.png)

###### fonte: survivingwithandroid.com

* Na utilização do protocolo MQTT com a IDE arduino e ESP8266 será necessário a importação da biblioteca **PubSubClient**.
* Instalação da biblioteca **PubSubClient**

![Instalação PubSubClient](../../../Imagens/ImportarBiblioteca.png)

* Detalhes e particularidades do código usando a IDE Arduino e ESP8266

```c++

#include <PubSubClient.h>

// Informações do servidor MQTT que será utilizado - MQTT Broker
const char* mqtt_server = "broker.mqtt-dashboard.com"; // Servidor MQTT - Broker gratuíto
// Identificação do canal virtual - topico que será utilizado
const char* topico = "Sistemas.Embarcados.Topico.Entrada"; 

//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "Servidot cloudmqtt.com";          //server
const char* mqttUser = "Usuario";                           //user
const char* mqttPassword = "************";                  //password
const int mqttPort = 00000;                                 //port
const char* topico = "Sistemas.Embarcados.Topico.Entrada";  //tópico que sera assinado

WiFiClient clienteWIFI;
// Criação do cliente MQTT para assinaturas (subscribing) e publicações (publishing)
PubSubClient clienteMQTT(clienteWIFI);

// Método que monitora o recebimento de mensagens do broker MQTT (Servidor MQTT)
void callback(char* topico, byte* payload, unsigned int tamanho) {

  //Seu código
  
}

// Conexão com o servidor MQTT para troca de mensagens
void conectaMQTT() {
  // Loop ate conexao
  while (!clienteMQTT.connected()) {
    Serial.print("Aguardando conexao MQTT...");
    if (clienteMQTT.connect(macAddress, mqttUser, mqttPassword)) {
      Serial.print("MQTT conectado - MAC address: ");
      Serial.println(macAddress);
      //faz subscribe automatico no topico
      clienteMQTT.subscribe(topico);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" tentando reconectar em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {

  // Conexão com broker no servidor MQTT
  clienteMQTT.setServer(mqttServer, mqttPort);
  // Definição do procedimento de recebimento de mensagens
  clienteMQTT.setCallback(callback);
  
}

void loop() {

  // mantem a conexão com o servidor (broker) ativa para troca de mensagens
  if (!clienteMQTT.connected()) {
    conectaMQTT();
  }
  // Thread de vericação de recebimento de mensagens
  clienteMQTT.loop();
  
}

```