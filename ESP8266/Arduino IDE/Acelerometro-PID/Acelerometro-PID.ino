#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "PID_v1.h"

#define PWM_B  2
#define PWM_A  2
#define AI2   16
#define AI1   13
#define BI1   12
#define BI2   14

#define VEL_MIN_ABS 50
#define FATOR_ESQUERDA 0.5
#define FATOR_DIREITA 0.5
#define PWM_MAX 1023
#define OFFSET_DESLIGA 30

MPU6050 mpu;
// controles MPU e variaveis de status
bool dmpPronto = false;
uint8_t mpuStatusInterrupcao;
uint8_t statusDispositivo;
uint16_t tamanhoInformacao;
uint16_t contadorFIFO;
uint8_t bufferFIFO[64];

// variaveis de orientacao e movimento
Quaternion q; // [w, x, y, z]
VectorFloat gravidade; // [x, y, z] vetor da forca da gravidade
float ypr[3]; // [yaw, pitch, roll]

//PID
double setpoint = 98;
double angulo, torque;
//Ajustar os parametros para o projeto em desenvolvimento
double Kp = 50;
double Ki = 0;
double Kd = 0 ;
PID pid(&angulo, &torque, &setpoint, Kp, Ki, Kd, DIRECT);

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
  Wire.begin();
  Serial.println("Inicializando dispositivos I2C...");
  mpu.initialize();
  Serial.println("Teste de conexao com o dispositivo MPU5060...");
  Serial.println(mpu.testConnection() ? "Conexao com sucesso com o MPU6050." : "A conexao com o MPU6050 falhou.");
  statusDispositivo = mpu.dmpInitialize();
  //Colocar aqui os dados obtidos na calibração do MPU 6050, cada configuração tem o seus próprios valores
  mpu.setXAccelOffset(203);
  mpu.setYAccelOffset(-3919);
  mpu.setZAccelOffset(904);
  mpu.setXGyroOffset(17);
  mpu.setYGyroOffset(169);
  mpu.setZGyroOffset(-12);
  if (statusDispositivo == 0) {
    // ligar o DMP (Digital motion Processor)
    mpu.setDMPEnabled(true);
    dmpPronto = true;
    tamanhoInformacao = mpu.dmpGetFIFOPacketSize();
    //setup PID
    pid.SetMode(AUTOMATIC);
    pid.SetSampleTime(3);
    pid.SetOutputLimits(-100, 100);

  } else {
    Serial.print("Inicializacao DMP falhou (codigo ");
    Serial.print(statusDispositivo);
    Serial.println(")");
  }
}

void loop() {
  ESPserialEvent();
  if (infoCompleta) {
    pid.SetTunings(Kp, Ki, Kd);
    infoCompleta = false;
  }
  if (!dmpPronto) {
    return;
  }
  // Espera pela interrupcao do MPU ou informacao extra
  while (contadorFIFO < tamanhoInformacao) {
    contadorFIFO = mpu.getFIFOCount();
    Serial.print("(kP= ");
    Serial.print(Kp);
    Serial.print(", kI= ");
    Serial.print(Ki);
    Serial.print(", kD= ");
    Serial.print(Kd);
    Serial.print(")\tÂngulo: ");
    Serial.print(angulo);
    if (angulo > (setpoint - OFFSET_DESLIGA) && angulo < (setpoint + OFFSET_DESLIGA)) {
      pid.Compute();
      Serial.print("\tTorque: ");
      Serial.println(torque);
      mover(torque);
    } else {
      Serial.println("\tDesligado");
      mover(0);
    }
  }
  if (contadorFIFO >= 1024) {
    mpu.resetFIFO();
    contadorFIFO = mpu.getFIFOCount();
    Serial.println(F("FIFO overflow!"));
  } else {
    // espera pelo tamanho da informacao correta
    while (contadorFIFO < tamanhoInformacao) {
      contadorFIFO = mpu.getFIFOCount();
    }
    mpu.getFIFOBytes(bufferFIFO, tamanhoInformacao);
    contadorFIFO -= tamanhoInformacao;
  }
  mpu.dmpGetQuaternion(&q, bufferFIFO);
  mpu.dmpGetGravity(&gravidade, &q);
  mpu.dmpGetYawPitchRoll(ypr, &q, &gravidade);
  //Utilização do Roll como valor do ângulo considerando montagem do acelerômetro
  angulo = ypr[2] * RAD_TO_DEG; // Converte o valor ypr[2] para ângulo em graus
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
