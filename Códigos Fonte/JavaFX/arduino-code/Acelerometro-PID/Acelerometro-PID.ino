#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "PID_v1.h"
#include "Ultrasonic.h"

#define PWM_B  2
#define PWM_A  2
#define AI2   16
#define AI1   13
#define BI1   12
#define BI2   14

#define TRIGGER_PIN  0
#define ECHO_PIN     15

#define VEL_MIN_ABS 40
#define PWM_MAX 1023
#define OFFSET_DESLIGA 30

Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

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
double setpoint = 97.5;
double angulo, distancia, torque;
//Ajustar os parametros para o projeto em desenvolvimento
double Kp = 70;
double Ki = 500;
double Kd = 2 ;
PID pid(&angulo, &torque, &setpoint, Kp, Ki, Kd, DIRECT);

float fatorEsquerda = 0.5;
float fatorDireita = 0.5;
boolean infoCompleta = false;
char fimInformacao = '*';
boolean conectaExterno = true;

unsigned long contador;    // controle de tempo para amostragem da distância
unsigned long intervalo = 50;     // Tempo em ms do intervalo a ser executado

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
    pid.SetSampleTime(5);
    pid.SetOutputLimits(-100, 100);
    contador = millis();
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
  if (millis() - contador > intervalo) {
    distancia = ultrasonic.convert(ultrasonic.timing(), Ultrasonic::CM);
    Serial.print("Avaliando distância: ");
    Serial.println(distancia);

    contador = millis();
  }
  // Espera pela interrupcao do MPU ou informacao extra
  while (contadorFIFO < tamanhoInformacao) {
    contadorFIFO = mpu.getFIFOCount();
    Serial.print(conectaExterno ? "" : "F. esquerdo= ");
    Serial.print(fatorEsquerda);
    Serial.print(conectaExterno ? ";" : "F. esquerdo= ");
    Serial.print(fatorDireita);
    Serial.print(conectaExterno ? ";" : "(kP= ");
    Serial.print(Kp);
    Serial.print(conectaExterno ? ";" : ", kI= ");
    Serial.print(Ki);
    Serial.print(conectaExterno ? ";" : ", kD= ");
    Serial.print(Kd);
    Serial.print(conectaExterno ? ";" : ")\tÂngulo: ");
    Serial.print(angulo);
    Serial.print(conectaExterno ? ";" : "\tTorque: ");
    Serial.print(torque);
    Serial.print(conectaExterno ? ";" : ")\tDistância: ");
    Serial.println(distancia);
    if (angulo > (setpoint - OFFSET_DESLIGA) && angulo < (setpoint + OFFSET_DESLIGA)) {
      pid.Compute();
      mover(torque);
    } else {
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
    analogWrite(PWM_A, velocidadeReal * fatorEsquerda);
    analogWrite(PWM_B, velocidadeReal * fatorDireita);
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
      case 'a':
        fatorEsquerda = Serial.parseFloat();
        break;
      case 'b':
        fatorDireita = Serial.parseFloat();
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
      case 't':
        pid.SetSampleTime(Serial.parseInt());
        break;
    }
    infoCompleta = (c == fimInformacao) || (c == '\n');
  }
}
