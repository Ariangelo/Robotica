#
# Exemplo MQTT publisher usando paho-mqtt
#

import sys
import time
import paho.mqtt.client as mqtt

#definicoes: 
broker = 'm16.cloudmqtt.com'
portaBroker = 11124 #1883
usuario = 'lsqvkvqh'
senha = 'rc9HDex9Z_SF'
topico = 'Sistemas.Embarcados.Topico.Entrada' #dica: o nome do topico deve ser 'unico', 

#Callback - conexao ao broker realizada
def on_connect(client, userdata, flags, rc):
    print('[STATUS] Conectado ao Broker. Resultado de conexao: ' + str(rc))
   
#programa principal:
def main():
    try:
        client = mqtt.Client() 
        client.username_pw_set(usuario, senha)		
        client.on_connect = on_connect
        client.connect(broker, portaBroker)
        client.loop_start()
        seq = 1
        while True:
            payload = '1' if seq % 2 == 0 else '0'
            client.publish(topico, payload)
            print('evento publicado: payload= {}'.format(payload))
            seq += 1
            time.sleep(1)
    except KeyboardInterrupt:
        print('\nCtrl+C pressionado, encerrando aplicacao e saindo...')
        sys.exit(0)
    
if __name__ == '__main__':
    main()    
