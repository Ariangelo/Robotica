#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

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
  //Colocar aqui os dados obtidos na calibração do MPU 6050, cada configuração tem o seus próprios valores
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
  //Utilização do Roll como valor do ângulo considerando montagem do acelerômetro
  angulo = ypr[2] * RAD_TO_DEG; // Converte o valor ypr[2] para ângulo em graus  
  Serial.print("Ângulo obtido: ");
  Serial.println(angulo);
}
