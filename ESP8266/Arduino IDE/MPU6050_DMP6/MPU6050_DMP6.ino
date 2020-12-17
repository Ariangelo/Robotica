#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

#define INTERRUPT_PIN 15 //D8

MPU6050 mpu;
// controles MPU e variaveis de status
bool dmpPronto = false;
uint8_t mpuStatusInterrupcao;
uint8_t statusDispositivo;
uint16_t tamanhoInformacao;
uint16_t contadorFifo;
uint8_t bufferFIFO[64];

// variaveis de orientacao e movimento
Quaternion q; // [w, x, y, z]
uint8_t infoSensor[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };
volatile bool interrupcaoMPU = false;

void infoPronta() {
  interrupcaoMPU = true;
}

void setup() {
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. 
  Serial.begin(115200);
  Serial.println("Inicializando dispositivos I2C...");
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);
  Serial.println("Teste de conexao com o dispositivo MPU5060...");
  Serial.println(mpu.testConnection() ? "Conexao com sucesso com o MPU6050." : "A conexao com o MPU6050 falhou.");

  // Sequencia de comandos para esvaziar e prepar o buffer de informações
  while (Serial.available() && Serial.read());
  while (!Serial.available());
  while (Serial.available() && Serial.read());

  statusDispositivo = mpu.dmpInitialize();
  if (statusDispositivo == 0) {
    // ligar o DMP (Digital motion Processor)
    mpu.setDMPEnabled(true);
    // habilitar o ESP para detectar interrupcoes
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), infoPronta, RISING);
    mpuStatusInterrupcao = mpu.getIntStatus();
    dmpPronto = true;
    tamanhoInformacao = mpu.dmpGetFIFOPacketSize();
  } else {
    Serial.print("Inicializacao DMP falhou (codigo ");
    Serial.print(statusDispositivo);
    Serial.println(")");
  }
}

void loop() {
  if (!dmpPronto) return;
  // Espera pela interrupcao do MPU ou informacao extra
  while (!interrupcaoMPU && contadorFifo < tamanhoInformacao) {
    if (interrupcaoMPU && contadorFifo < tamanhoInformacao) {
      contadorFifo = mpu.getFIFOCount();
    }
  }
  interrupcaoMPU = false;
  mpuStatusInterrupcao = mpu.getIntStatus();
  contadorFifo = mpu.getFIFOCount();
  // verifica condicao de overflow
  if ((mpuStatusInterrupcao & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT)) || contadorFifo >= 1024) {
    mpu.resetFIFO();
    contadorFifo = mpu.getFIFOCount();
    Serial.println("FIFO overflow!");
  } else if (mpuStatusInterrupcao & _BV(MPU6050_INTERRUPT_DMP_INT_BIT)) {
    // espera pelo tamanho da informacao correta
    while (contadorFifo < tamanhoInformacao) {
      contadorFifo = mpu.getFIFOCount();
    }
    mpu.getFIFOBytes(bufferFIFO, tamanhoInformacao);
    contadorFifo -= tamanhoInformacao;
    // Disponibiliza a informacao do sensor para processamento
    infoSensor[2] = bufferFIFO[0];
    infoSensor[3] = bufferFIFO[1];
    infoSensor[4] = bufferFIFO[4];
    infoSensor[5] = bufferFIFO[5];
    infoSensor[6] = bufferFIFO[8];
    infoSensor[7] = bufferFIFO[9];
    infoSensor[8] = bufferFIFO[12];
    infoSensor[9] = bufferFIFO[13];
    Serial.write(infoSensor, 14);
    infoSensor[11]++; // ajusta contador com loop ate 0xFF
  }
}
