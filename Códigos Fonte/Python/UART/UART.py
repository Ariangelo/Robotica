#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import time
import serial.tools.list_ports
import keyboard
from serial import Serial

passo = 200

def portasSeriais():
    #Lista as portas seriais
    listaPortas = serial.tools.list_ports.comports()
    conectadas = []
    for p in listaPortas:
        conectadas.append(p.device)
    return conectadas
    
def keyPressed(e):
    global passo
    tecla = ', '.join(str(code) for code in keyboard._pressed_events)
    if tecla == '72': #Seta para cima
        if (passo < 200):
            passo += 50
        else:
            passo = 200
        print(u'Passo: {}'.format(passo))
    elif tecla == '80': #Seta para baixo
        if (passo > 50):
            passo -= 50
        else:
            passo = 50
        print(u'Passo: {}'.format(passo))
    elif tecla == '75': #Seta para esquerda
        serial.write(u'x{}\n'.format(passo).encode()) 
    elif tecla == '77': #Seta para direita
        serial.write(u'x-{}\n'.format(passo).encode())
    elif tecla == '71': #Home
        serial.write(b'h\n')
    else:
        serial.write(b'0')

if __name__ == "__main__":
    print(u'Portas disponíveis -> ' + ', '.join(portasSeriais()))
    porta = input("Digite a porta desejada: ")
    try:
        try:
            serial = Serial(porta)
            print('Conectado a porta {}'.format(porta))
            print(u'Use as setas para movimentar o motor')
        except:
            print(u'Nao foi possível abrir a porta {}'.format(porta))
            sys.exit(1)
        keyboard.hook(keyPressed)
        keyboard.wait()
    except:
        print('Sistema encerrado!')
