#ifndef DC_Motor_h
#define DC_Motor_h

#include "Arduino.h"

class DC_Motor { 
    private:
        int pin1, pin2, pwmPin, velocidadeMotor;
            
    public:
        DC_Motor(int pin_1,int pin_2, int pwm_Pin);
        void horario();
        void antiHorario();
        void velocidade(int velocidade);  
        void parar();
};
  
#endif
