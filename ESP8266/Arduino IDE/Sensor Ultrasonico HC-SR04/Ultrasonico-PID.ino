#include "PID_v1.h"
#include <Ultrasonic.h>

#define PWM_B  2
#define PWM_A  2
#define AI2   16
#define AI1   13
#define BI1   12
#define BI2   14

#define VEL_MIN_ABS 60
#define FATOR_ESQUERDA 0.5
#define FATOR_DIREITA 0.5
#define PWM_MAX 1023

#define TRIGGER_PIN  0
#define ECHO_PIN     15

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

//PID
double setpoint = 20;
double distancia, torque;
//Ajustar os parametros para o projeto em desenvolvimento
double Kp = 50;
double Ki = 0;
double Kd = 0;
PID pid(&distancia, &torque, &setpoint, Kp, Ki, Kd, DIRECT);

boolean infoCompleta = false;
char fimInformacao = '*';

void setup() {
  pinMode(PWM_A, OUTPUT);
  pinMode(AI1, OUTPUT);
  pinMode(AI2, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(BI1, OUTPUT);
  pinMode(BI2, OUTPUT);

  Serial.begin(115200);

  pid.SetMode(AUTOMATIC);
  pid.SetSampleTime(100);
  pid.SetOutputLimits(-100, 100);
}

void loop() {
  ESPserialEvent();
  if (infoCompleta) {
    pid.SetTunings(Kp, Ki, Kd);
    infoCompleta = false;
  }
  distancia = ultrasonic.convert(ultrasonic.timing(), Ultrasonic::CM);
  Serial.print(FATOR_ESQUERDA);
  Serial.print(";");
  Serial.print(FATOR_DIREITA);
  Serial.print(";");
  Serial.print(Kp);
  Serial.print(";");
  Serial.print(Ki);
  Serial.print(";");
  Serial.print(Kd);
  Serial.print(";");
  Serial.println(distancia);
  pid.Compute();
  mover(torque);
}

void mover(int velocidade) {
  int velocidadeReal = abs(velocidade * PWM_MAX / 100);
  digitalWrite(AI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(AI2, velocidade > 0 ? LOW : HIGH);
  digitalWrite(BI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(BI2, velocidade > 0 ? LOW : HIGH);
  if (abs(velocidade) > VEL_MIN_ABS) {
    analogWrite(PWM_A, velocidadeReal * FATOR_ESQUERDA);
    analogWrite(PWM_B, velocidadeReal * FATOR_DIREITA);
  } else {
    analogWrite(PWM_A, 0);
    analogWrite(PWM_B, 0);
  }
}

void ESPserialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
      case 'p':
        Kp = Serial.parseFloat();
        break;
      case 'i':
        Ki = Serial.parseFloat();
        break;
      case 'd':
        Kd = Serial.parseFloat();
        break;
      case 's':
        setpoint = Serial.parseFloat();
        break;
    }
    infoCompleta = (c == fimInformacao) || (c == '\n');
  }
}
