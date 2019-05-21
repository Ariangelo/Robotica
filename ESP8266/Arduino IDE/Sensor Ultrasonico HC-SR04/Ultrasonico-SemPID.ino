#include <Ultrasonic.h>

#define PWM_B  2
#define PWM_A  2
#define AI2   16
#define AI1   13
#define BI1   12
#define BI2   14

#define FATOR_ESQUERDA 0.5
#define FATOR_DIREITA 0.5
#define PWM_MAX 1023

#define TRIGGER_PIN  0
#define ECHO_PIN     15

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

void setup() {
  pinMode(PWM_A, OUTPUT);
  pinMode(AI1, OUTPUT);
  pinMode(AI2, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(BI1, OUTPUT);
  pinMode(BI2, OUTPUT);
  
  Serial.begin(115200);
}

void loop() {
  float distancia = ultrasonic.convert(ultrasonic.timing(), Ultrasonic::CM);
  Serial.print("Distância: ");
  Serial.print(distancia);
  Serial.println("cm");
  mover(distancia < 20 ? 100 : -100);
  delay(100);
}

void mover(int velocidade) {
  int velocidadeReal = abs(velocidade * PWM_MAX / 100);
  digitalWrite(AI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(AI2, velocidade > 0 ? LOW : HIGH);
  digitalWrite(BI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(BI2, velocidade > 0 ? LOW : HIGH);
  analogWrite(PWM_A, velocidadeReal * FATOR_ESQUERDA);
  analogWrite(PWM_B, velocidadeReal * FATOR_DIREITA);
}
