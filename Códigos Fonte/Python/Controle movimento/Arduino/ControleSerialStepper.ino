#include <MyStepper.h>

#define X_SENSOR_INICIAL 14
#define X_SENSOR_FINAL 12

struct Configuration {
  int initialStep = 1;
  int maxX = 0;
  int speed = 1250;
  int acceleration = 2500;
};

Configuration confValues;
boolean infoCompleta = false;
char fimInformacao = '*';
boolean calibracao = false;
int deslocamentoX = 0;
int maxStep = 0;
bool movendo = false;

MyStepper stepperX(AccelStepper::DRIVER, 2, 0); // Defaults to AccelStepper::DRIVER (2 pins) on 2, 3

void setup() {
  Serial.begin(9600);
  pinMode(X_SENSOR_INICIAL, INPUT);
  pinMode(X_SENSOR_FINAL, INPUT);
  stepperX.setAcceleration(confValues.acceleration);
  stepperX.setMaxSpeed(confValues.speed);
}

void loop() {
  ESPserialEvent();
  if (calibracao) {
    if (digitalRead(X_SENSOR_FINAL) == HIGH) {
      stepperX.setCurrentPosition(0);
      stepperX.moveToDistance(-500);
    }
    if (digitalRead(X_SENSOR_INICIAL) == HIGH) {
      Serial.println("Motor parado");
      stepperX.stop();
      maxStep = stepperX.getCurrentPositionDistance() * -1;
      Serial.print("Valor total do percurso: ");
      Serial.println(maxStep);
      stepperX.setCurrentPosition(0);
      stepperX.runToNewDistance(confValues.initialStep + 1);
      stepperX.runToNewDistance(confValues.initialStep);
      stepperX.setCurrentPosition(0);
      calibracao = false;
    } else {
      stepperX.run();
    }
  }
  if (infoCompleta) {
    if (deslocamentoX >= 0 || calibracao) {
      if (calibracao) {
        Serial.println("Calibrando");
        stepperX.moveToDistance(500);
      }
      else
      {
        if (deslocamentoX > maxStep) {
          deslocamentoX = maxStep;
        }
        movendo = true;
        Serial.println(deslocamentoX);
        stepperX.moveToDistance(deslocamentoX);
      }
    }
    infoCompleta = false;
  }
  if (stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
  if (stepperX.distanceToGo() == 0 && movendo) {
    Serial.println("ok");
    movendo = false;
  }
}

void ESPserialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    Serial.flush();
    switch (c)
    {
      case 'm':
        stepperX.setMicroStep(Serial.parseFloat());
        break;
      case 'd':
        stepperX.setDistancePerRevolution(Serial.parseFloat());
        break;
      case 'r':
        stepperX.setStepsPerRevolution(Serial.parseInt());
        break;
      case 'a':
        confValues.acceleration = Serial.parseInt();
        stepperX.setAcceleration(confValues.acceleration);
        break;
      case 's':
        confValues.speed = Serial.parseInt();
        stepperX.setMaxSpeed(confValues.speed);
        break;
      case 'x':
        deslocamentoX = Serial.parseInt();
        break;
      case 'c':
        calibracao = true;
        break;
      case 'h':
        deslocamentoX = 0;
        break;
    }
    infoCompleta = (c == fimInformacao) || (c == '\n');
  }
}
