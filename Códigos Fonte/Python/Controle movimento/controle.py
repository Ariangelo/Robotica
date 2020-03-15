#!/usr/bin/env python
# -*- coding: utf-8 -*-

import yaml
import sys
import time
import serial.tools.list_ports
import keyboard
from serial import Serial
from threading import Thread

parar = False

# Classe Movimento utilizada para controle dos movimentos
# Implementado como uma Thread para possibilitar controles
# em segundo plano, como por exemplo retornar ao início <Home>
class Movimento(Thread):

    def __init__ (self, comandos):
        Thread.__init__(self)
        self.comandos = comandos

    def run(self):
        global parar
        i = 0
        while (i < len(self.comandos['movements'])) and not parar:
            movimento = self.comandos['movements'][i]
            print('Movimento [{}]'.format(i))
            print('Aceleração = {}'.format(movimento['acceleration']))
            serial.write(u'a{}\n'.format(movimento['acceleration']).encode())
            print('Velocidade  = {}'.format(movimento['speed']))
            serial.write(u's{}\n'.format(movimento['speed']).encode())
            print('Deslocamento  = {}'.format(movimento['value']))
            serial.write(u'x{}\n'.format(movimento['value']).encode())
            serial.readline()
            time.sleep(movimento['wait'])
            i += 1
            if (i == len(self.comandos['movements']) and self.comandos['repeat']):
                i = 0
            print('----------')

# Procedimento utilizado para listar portas seriais do sistema
def portasSeriais():
    # Lista as portas seriais
    listaPortas = serial.tools.list_ports.comports()
    conectadas = []
    for p in listaPortas:
        conectadas.append(p.device)
    return conectadas

# Procedimento para possibilitar a entrada direta pelo teclado
def keyPressed(e):
    global parar
    tecla = ', '.join(str(code) for code in keyboard._pressed_events)
    if tecla == '71':  # Home
        parar = True
        serial.write(b'h\n')
    elif tecla == '46':  # letra c -> calibrar
        serial.write(b'c\n')
    elif tecla == '38':  # letra l -> carregar arquivo de comandos
        parar = False
        # Obtenção dos dados do arquivo YAML
        with open('comandos.yaml') as file:
            comandos = yaml.load(file, Loader=yaml.FullLoader)

        print('Configurando:')
        print('Micro Steps = {}'.format(comandos['microSteps']))
        serial.write(u'm{}\n'.format(comandos['microSteps']).encode())
        print('Steps por Volta= {}'.format(comandos['stepPerRevolution']))
        serial.write(u'r{}\n'.format(comandos['stepPerRevolution']).encode())
        print('Distância por volta = {}'.format(comandos['distancePerRevolution']))
        serial.write(u'd{}\n'.format(comandos['distancePerRevolution']).encode())
        print('Aceleração = {}'.format(comandos['acceleration']))
        serial.write(u'a{}\n'.format(comandos['acceleration']).encode())
        print('Velocidade Máxima = {}'.format(comandos['speed']))
        serial.write(u's{}\n'.format(comandos['speed']).encode())
        # Ativação do procedimento em segundo plano
        threadMovimento = Movimento(comandos)
        threadMovimento.start() 

if __name__ == "__main__":
    print(u'Portas disponíveis -> ' + ', '.join(portasSeriais()))
    porta = input("Digite a porta desejada: ")
    try:
        try:
            serial = Serial(porta)
            print('Conectado a porta {}'.format(porta))
            print(u'Use as setas para movimentar o motor')
            serial.reset_input_buffer()
        except:
            print(u'Nao foi possível abrir a porta {}'.format(porta))
            sys.exit(1)
        keyboard.hook(keyPressed)
        keyboard.wait()
    except:
        print('Sistema encerrado!')
