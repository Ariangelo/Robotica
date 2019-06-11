MQTT utizando ESP8266 e Python
------
* Descrição do Projeto

Descrição do código fonte da biblioteca cliente Eclipse **Paho MQTT Python**, que implementa as versões 3.1 e 3.1.1 do protocolo MQTT.

A **Paho MQTT Python** fornece uma classe de cliente que permite que os aplicativos se conectem a um broker do MQTT para publicar (publish) mensagens e para assinar (subscribe) tópicos e também receber mensagens publicadas.

Suporta o Python 2.7.9+ ou 3.4+, com suporte limitado para o Python 2.7 antes do 2.7.9.

Paho é um projeto da Eclipse Foundation.

* Instalação
A última versão estável está disponível no Python Package Index (PyPi) e pode ser instalada usando

```
> pip install paho-mqtt
```

* Detalhes do código Python e suas particularidades

```Python

import paho.mqtt.client as mqtt #importação da biblioteca paho

#definicoes: 
broker = 'SeuMQTT.cloudmqtt.com'
portaBroker = 00000 # Substituir pelo atribuido pelo cloudmqtt
usuario = 'Usuario'
senha = 'Senha'
topico = 'Sistemas.Embarcados.Topico.Entrada' #dica: o nome do topico deve ser 'unico', 

#Callback - conexão ao broker realizada
def on_connect(client, userdata, flags, rc):
    print("[STATUS] Conectado ao Broker. Resultado de conexao: " + str(rc))

```
* Utilização e definição do cliente MQTT para troca de mensagens

```Python

#programa principal:
def main():
    try:
        client = mqtt.Client()  # instancia o cliente MQTT
        client.username_pw_set(usuario, senha) # informa as credenciais
        client.on_connect = on_connect # define callback de conexão para troca de mensagens
        client.connect(broker, portaBroker) # conexão com o servidor MQTT - broker
        client.loop_start() # thread de monitoramento para troca de mensagens
        seq = 1
        while True:
            payload = '1' if seq % 2 == 0 else '0'
            client.publish(topico, payload) # publicaçõa do Canal Virtual - Tópico
            print('evento publicado: payload= {}'.format(payload))
            seq += 1
            time.sleep(1) # muda o estado do led conectado no cliente - ESP8266 a cada 1 segundo
    except KeyboardInterrupt:
        print("\nCtrl+C pressionado, encerrando aplicacao e saindo...")
        sys.exit(0)
        
```
