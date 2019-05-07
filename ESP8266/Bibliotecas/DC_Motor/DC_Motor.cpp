#include "DC_Motor.h"

DC_Motor::DC_Motor(int pin_1,int pin_2, int pwm_Pin) {
    pin1 = pin_1;
    pin2 = pin_2; 
    pwmPin = pwm_Pin;
    
    pinMode(pin1,OUTPUT);                                                 
    pinMode(pin2,OUTPUT);                                              
    pinMode(pwmPin,OUTPUT); 
    
    velocidade(50);
    parar();
} 
     
void DC_Motor::horario() {
    digitalWrite(pin1,HIGH);
    digitalWrite(pin2,LOW);  
}

void DC_Motor::antiHorario() {
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,HIGH);                                               
}
                      
void DC_Motor::parar() {
    digitalWrite(pin1,LOW);
    digitalWrite(pin2,LOW);                                               
}   
  
void DC_Motor::velocidade(int velocidade) {
    velocidadeMotor = map(velocidade, 0, 100, 0, PWMRANGE);  
    analogWrite(pwmPin, velocidadeMotor);
}
