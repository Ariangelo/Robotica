#include <MyStepper.h>

#define X_SENSOR_INICIAL 14
#define X_SENSOR_FINAL 12

struct Configuration {
  int initialStep = 1;
  int maxX = 0;
  int mSpeed = 600;
};

Configuration confValues;
boolean infoCompleta = false;
boolean calibracao = false;
int deslocamentoX = 0;
int maxStep = 0;

MyStepper stepperX(AccelStepper::DRIVER, 2, 0); // Defaults to AccelStepper::DRIVER (2 pins) on 2, 3

void setup() {
  Serial.begin(9600);

  pinMode(X_SENSOR_INICIAL, INPUT);
  pinMode(X_SENSOR_FINAL, INPUT);

  stepperX.setMaxSpeed(confValues.mSpeed);
  stepperX.setAcceleration(5000);
  stepperX.setMicroStep(1);
  stepperX.setStepsPerRevolution(200);
  stepperX.setDistancePerRevolution(8);
  stepperX.setAnglePerRevolution(90);
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
        Serial.println(deslocamentoX);
        stepperX.moveToDistance(deslocamentoX);
      }
    }
    infoCompleta = false;
  }
  if (stepperX.distanceToGo() != 0) {
    stepperX.run();
  }
}

void ESPserialEvent() {
  while (Serial.available()) {
    char c = Serial.read();
    switch (c)
    {
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
    infoCompleta = c == '\n';
  }
}
