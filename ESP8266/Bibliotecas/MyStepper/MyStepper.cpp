#include "MyStepper.h"

MyStepper::MyStepper(
	uint8_t interface, 
	uint8_t pin1, 
	uint8_t pin2, 
	uint8_t pin3, 
	uint8_t pin4, 
	bool enable) : 
	AccelStepper(interface, pin1, pin2, pin3, pin4, enable) {} 

void MyStepper::setMicroStep(int value) {
	microSteps = value;
}

void MyStepper::setStepsPerRevolution(int value) {
	stepsPerRevolution = value;
}

void MyStepper::setDistancePerRevolution(float value) {
	distancePerRevolution = value;
}

float MyStepper::getCurrentPositionDistance() {
	return currentPosition() / (microSteps * stepsPerRevolution / distancePerRevolution);
}

void MyStepper::moveToDistance(float value) {
	moveTo(value * microSteps * stepsPerRevolution / distancePerRevolution);
}

void MyStepper::runToNewDistance(float value) {
    moveToDistance(value);
    runToPosition();
}

void MyStepper::setAnglePerRevolution(float value) {
	anglePerRevolution = value;
}

void MyStepper::moveToAngle(float value) {
	moveTo(value * microSteps * stepsPerRevolution / anglePerRevolution);
}

void MyStepper::runToNewAngle(float value) {
    moveToAngle(value);
    runToPosition();
}
