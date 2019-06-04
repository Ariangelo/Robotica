#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "PID_v1.h"
#include "EEPROM.h"
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

//Ajustar os parametros para o projeto em desenvolvimento
struct Configuration {
  float fatorEsquerda;
  float fatorDireita;
  double Kp;
  double Ki;
  double Kd;
  double setpoint;
  int tempoAmostragem;
};

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
double setpoint, objetivo;
double angulo, distancia, torque = 0;
PID pid(&angulo, &torque, &setpoint, 1, 0, 0, DIRECT);

boolean infoCompleta = false;
char fimInformacao = '*';
boolean conectaExterno = true;

Configuration confValues;

unsigned long contador;    // controle de tempo para amostragem da distância
unsigned long intervalo = 2000;     // Tempo em ms do intervalo a ser executado

bool parado = true, frente = false, voltar = false;

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
  EEPROM.begin(48);//Inicia a EEPROM com o tamanho da informação.
  getEEPROM();
  //Colocar aqui os dados obtidos na calibração do MPU 6050, cada configuração tem o seus próprios valores
  mpu.setXAccelOffset(-336);
  mpu.setYAccelOffset(-135);
  mpu.setZAccelOffset(1257);
  mpu.setXGyroOffset(92);
  mpu.setYGyroOffset(-8);
  mpu.setZGyroOffset(-33);
  if (statusDispositivo == 0) {
    // ligar o DMP (Digital motion Processor)
    mpu.setDMPEnabled(true);
    dmpPronto = true;
    tamanhoInformacao = mpu.dmpGetFIFOPacketSize();
    //setup PID
    setpoint = confValues.setpoint;
    objetivo = setpoint;
    pid.SetMode(AUTOMATIC);
    pid.SetTunings(confValues.Kp, confValues.Ki, confValues.Kd);
    pid.SetSampleTime(confValues.tempoAmostragem);
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
    gravaEEPROM();
    pid.SetTunings(confValues.Kp, confValues.Ki, confValues.Kd);
    pid.SetSampleTime(confValues.tempoAmostragem);
    infoCompleta = false;
  }
  if (!dmpPronto) {
    return;
  } 
  /*
  if (millis() - contador > intervalo) {
    if (parado) {
      objetivo = confValues.setpoint;
      parado = false;
      frente = true;
    }
    if (frente) {
      objetivo = confValues.setpoint - 0.9;
      frente = false;
      voltar = true;
    }
    if (voltar) {
      objetivo = confValues.setpoint + 1.0;
      voltar = false;
      parado = true;
    }
    contador = millis();
  }
  ajustaSetpoint();
  */
  // Espera pela interrupcao do MPU ou informacao extra
  while (contadorFIFO < tamanhoInformacao) {
    contadorFIFO = mpu.getFIFOCount();
    Serial.print(conectaExterno ? "" : "F. esquerdo= ");
    Serial.print(confValues.fatorEsquerda);
    Serial.print(conectaExterno ? ";" : "F. esquerdo= ");
    Serial.print(confValues.fatorDireita);
    Serial.print(conectaExterno ? ";" : "(kP= ");
    Serial.print(confValues.Kp);
    Serial.print(conectaExterno ? ";" : ", kI= ");
    Serial.print(confValues.Ki);
    Serial.print(conectaExterno ? ";" : ", kD= ");
    Serial.print(confValues.Kd);
    Serial.print(conectaExterno ? ";" : ")\tÂngulo: ");
    Serial.print(angulo);
    Serial.print(conectaExterno ? ";" : ")\tSet point: ");
    Serial.print(confValues.setpoint);
    Serial.print(conectaExterno ? ";" : ")\tAmostragem: ");
    Serial.print(confValues.tempoAmostragem);
    Serial.print(conectaExterno ? ";" : "\tTorque: ");
    Serial.print(torque);
    Serial.print(conectaExterno ? ";" : ")\tDistância: ");
    Serial.println(distancia);
    if (angulo > (setpoint - OFFSET_DESLIGA) && angulo < (setpoint + OFFSET_DESLIGA)) {
      pid.Compute();
    } else {
      torque = 0;
    }
    mover(torque);
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

void ajustaSetpoint() {
  if (setpoint == objetivo) {
    return;
  }
  if (setpoint > objetivo) {
    setpoint -= 0.05;
  } else {
    setpoint += 0.05;
  }
}

void mover(int velocidade) {
  int velocidadeReal = abs(velocidade * PWM_MAX / 100);
  digitalWrite(AI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(AI2, velocidade > 0 ? LOW : HIGH);
  digitalWrite(BI1, velocidade > 0 ? HIGH : LOW);
  digitalWrite(BI2, velocidade > 0 ? LOW : HIGH);
  if (abs(velocidade) > VEL_MIN_ABS) {
    analogWrite(PWM_A, velocidadeReal * confValues.fatorEsquerda);
    analogWrite(PWM_B, velocidadeReal * confValues.fatorDireita);
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
        confValues.fatorEsquerda = Serial.parseFloat();
        break;
      case 'b':
        confValues.fatorDireita = Serial.parseFloat();
        break;
      case 'p':
        confValues.Kp = Serial.parseFloat();
        break;
      case 'i':
        confValues.Ki = Serial.parseFloat();
        break;
      case 'd':
        confValues.Kd = Serial.parseFloat();
        break;
      case 's':
        setpoint = Serial.parseFloat();
        confValues.setpoint = setpoint;
        break;
      case 't':
        confValues.tempoAmostragem = Serial.parseInt();
        break;
      case 'f':
        //Para frente
        setpoint = confValues.setpoint - 0.5;
        break;
      case 'c':
        //Para trás
        setpoint = confValues.setpoint + 1.0;
        break;
      case 'h':
        //Equilíbrio
        setpoint = confValues.setpoint;
        break;
    }
    infoCompleta = (c == fimInformacao) || (c == '\n');
  }
}

void getEEPROM() {
  EEPROM.get(0, confValues);
  if (confValues.tempoAmostragem == -1) {
    confValues.fatorEsquerda = 0.5;
    confValues.fatorDireita = 0.5;
    confValues.Kp = 70.0;
    confValues.Ki = 500.0;
    confValues.Kd = 2.0;
    confValues.setpoint = 103.0;
    confValues.tempoAmostragem = 5;
    gravaEEPROM();
  }
  mostrarEEPROM("Lendo EEPROM");
}

void gravaEEPROM() {
  EEPROM.put(0, confValues);
  EEPROM.commit();//Salva o dado na EEPROM.
  mostrarEEPROM("Gravando EEPROM");
}

void mostrarEEPROM (String titulo) {
  Serial.println(titulo);
  Serial.print("F. esquerdo= ");
  Serial.print(confValues.fatorEsquerda);
  Serial.print(" F. direito= ");
  Serial.print(confValues.fatorDireita);
  Serial.print(" (kP= ");
  Serial.print(confValues.Kp);
  Serial.print(", kI= ");
  Serial.print(confValues.Ki);
  Serial.print(", kD= ");
  Serial.print(confValues.Kd);
  Serial.print(", Set point= ");
  Serial.print(confValues.setpoint);
  Serial.print(", Tempo amostragem= ");
  Serial.println(confValues.tempoAmostragem);
}
