#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include<DC_Motor.h>

#define PWM_B  2
#define PWM_A  2
#define AI2   16
#define AI1   13
#define BI1   12
#define BI2   14

#define VELOCIDADE_MOTOR      50
#define TEMPO_MOTOR_LIGADO    15
#define LIMITE_FRENTE         80  
#define LIMITE_TRAS           100

DC_Motor motorEsquerdo(BI1, BI2, PWM_B);
DC_Motor motorDireito(AI1, AI2, PWM_A);

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
float angulo;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Inicializando dispositivos I2C...");
  mpu.initialize();
  Serial.println("Teste de conexao com o dispositivo MPU5060...");
  Serial.println(mpu.testConnection() ? "Conexao com sucesso com o MPU6050." : "A conexao com o MPU6050 falhou.");
  statusDispositivo = mpu.dmpInitialize();
  mpu.setXAccelOffset(-711);
  mpu.setYAccelOffset(1057);
  mpu.setZAccelOffset(1368);
  mpu.setXGyroOffset(14);
  mpu.setYGyroOffset(-22);
  mpu.setZGyroOffset(0);
  if (statusDispositivo == 0) {
    // ligar o DMP (Digital motion Processor)
    mpu.setDMPEnabled(true);
    dmpPronto = true;
    tamanhoInformacao = mpu.dmpGetFIFOPacketSize();
  } else {
    Serial.print("Inicializacao DMP falhou (codigo ");
    Serial.print(statusDispositivo);
    Serial.println(")");
  }
  motorEsquerdo.velocidade(VELOCIDADE_MOTOR);
  motorDireito.velocidade(VELOCIDADE_MOTOR);
}

void loop() {
  if (!dmpPronto) {
    return;
  }
  // Espera pela interrupcao do MPU ou informacao extra
  while (contadorFIFO < tamanhoInformacao) {
    contadorFIFO = mpu.getFIFOCount();
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
  angulo = ypr[2] * RAD_TO_DEG;
  
  Serial.print("Ângulo do veículo: ");
  Serial.print(angulo);

  if (angulo >= LIMITE_FRENTE && angulo <= LIMITE_TRAS) {
    parar();
  } else if (angulo < LIMITE_FRENTE) {
    Serial.println("Para frente");
    paraFrente(TEMPO_MOTOR_LIGADO);
  } else {
    Serial.println("Para trás");
    paraTras(TEMPO_MOTOR_LIGADO);
  }
}

void paraFrente(int tempoLigado) {
  motorEsquerdo.horario();
  motorDireito.horario();
  delay(tempoLigado);
  motorEsquerdo.parar();
  motorDireito.parar();
}

void paraTras(int tempoLigado) {
  motorEsquerdo.antiHorario();
  motorDireito.antiHorario();
  delay(tempoLigado);
  motorEsquerdo.parar();
  motorDireito.parar();
}

void parar() {
  motorEsquerdo.parar();
  motorDireito.parar();
}
