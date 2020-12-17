#include "ESP8266WiFi.h"
#include "DNSServer.h"
#include "ESP8266WebServer.h"
#include "WiFiManager.h"         //https://github.com/tzapu/WiFiManager
#include "PubSubClient.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
#include "PID_v1.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

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
double setpoint, objetivo, incrementoObjetivo;
double angulo, distancia, torque = 0;
PID pid(&angulo, &torque, &setpoint, 1, 0, 0, DIRECT);

boolean infoCompleta = false;
char fimInformacao = '*';
boolean conectaExterno = true;

Configuration confValues;

unsigned long contador;    // controle de tempo para amostragem da distância
unsigned long intervalo = 500;     // Tempo em ms do intervalo a ser executado

//informações do broker MQTT - Verifique as informações geradas pelo CloudMQTT
const char* mqttServer = "xxxxxxxx";   //server
const char* mqttUser = "xxxxxxx";      //user
const char* mqttPassword = "xxxxxxx";  //password
const int mqttPort = 1883;             //port
const char* topicoEntrada = "Sistemas.Embarcados.Topico.Entrada";  //tópico que sera assinado
const char* topicoSaida = "Sistemas.Embarcados.Topico.Saida";  //tópico que sera assinado
bool autenticacao = false;
String strMacAddress;
char macAddress[6];

WiFiClient clienteWIFI;
PubSubClient clienteMQTT(clienteWIFI);

StaticJsonDocument<200> doc;

// Metodo que monitora o recebimento de mensagens do broker MQTT
void callback(char* topico, byte* payload, unsigned int tamanho) {
  char info[32];
  Serial.print("[MSG RECEBIDA] Topico: ");
  Serial.print(topico);
  Serial.print(" / Mensagem: ");
  for (int i = 0; i < tamanho; i++) {
    info[i] = (char)payload[i];
  }
  DeserializationError erro = deserializeJson(doc, info);
  if (!erro) {
    serializeJson(doc, Serial);
    Serial.println();
    const char* tmpAcao = doc["acao"];
    char acao = tmpAcao[0];
    // Controla o movimento do Robô
    switch (acao) {
      case 'a': {
          float valor = doc["valor"];
          confValues.fatorEsquerda = valor;
          break;
        }
      case 'b': {
          float valor = doc["valor"];
          confValues.fatorDireita = valor;
          break;
        }
      case 's': {
          double valor = doc["valor"];
          confValues.setpoint = valor;
          setpoint = valor;
          break;
        }
      case 't': {
          int valor =  doc["valor"];
          confValues.tempoAmostragem = valor;
          break;
        }
      case 'p': {
          double valor =  doc["valor"];
          confValues.Kp = valor;
          break;
        }
      case 'i': {
          double valor =  doc["valor"];
          confValues.Ki = valor;
          break;
        }
      case 'd': {
          double valor =  doc["valor"];
          confValues.Kd = valor;
          break;
        }
      case 'g': {
          gravaEEPROM();
          break;
        }
      case 'v':
        objetivo = confValues.setpoint + 0.6;
        incrementoObjetivo = 0.01;
        break;
      case 'h':
        objetivo = confValues.setpoint;
        incrementoObjetivo = 0.1;
        break;
      case 'f':
        objetivo = confValues.setpoint - 0.9;
        incrementoObjetivo = 0.01;
        break;
    }
    infoCompleta = true;
  }
}

void conectaMQTT() {
  // Loop ate conexao
  while (!clienteMQTT.connected()) {
    Serial.print("Aguardando conexao MQTT...");
    if (autenticacao ? clienteMQTT.connect(macAddress, mqttUser, mqttPassword) : clienteMQTT.connect(macAddress)) {
      Serial.println("MQTT conectado");
      //faz subscribe automatico no topico
      clienteMQTT.subscribe(topicoEntrada);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(clienteMQTT.state());
      Serial.println(" tentando reconectar em 5 segundos.");
      //delay(5000);
    }
  }
}

void setup() {
  pinMode(PWM_A, OUTPUT);
  pinMode(AI1, OUTPUT);
  pinMode(AI2, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(BI1, OUTPUT);
  pinMode(BI2, OUTPUT);
  Serial.begin(115200);
  // Conexao to Wi-Fi
  Serial.print("Conectando ");
  //WiFiManager
  //Inicialização local. Uma vez que realizado, não há necessidade de informar credenciais novamente
  WiFiManager wifiManager;
  //Obtem o ssid e grava na eeprom e tenta se conectar
  //se não conectar, inicia um ponto de acesso com o nome especificado "AutoConnectAP"
  //e entra em um loop aguardando configuração
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("WiFi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Use este endereço para conectar ao ESP8266");
  Serial.println();
  strMacAddress = WiFi.macAddress();
  strMacAddress.toCharArray(macAddress, 6);
  // Conexao com broker no servidor MQTT
  clienteMQTT.setServer(mqttServer, mqttPort);
  // Definicao do procedimento de recebimento de mensagens
  clienteMQTT.setCallback(callback);

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
  if (!clienteMQTT.connected()) {
    conectaMQTT();
  }
  clienteMQTT.loop();
  ESPserialEvent();
  ajustaSetpoint();
  if (infoCompleta) {
    pid.SetTunings(confValues.Kp, confValues.Ki, confValues.Kd);
    pid.SetSampleTime(confValues.tempoAmostragem);
    pid.Compute();
    infoCompleta = false;
  }
  if (!dmpPronto) {
    return;
  }
  // Espera pela interrupcao do MPU ou informacao extra
  while (contadorFIFO < tamanhoInformacao) {
    contadorFIFO = mpu.getFIFOCount();
    String info = (conectaExterno ? "" : "F. esquerdo= ") + String(confValues.fatorEsquerda, 2) +
                  (conectaExterno ? ";" : "F. esquerdo= ") + String(confValues.fatorDireita, 2) +
                  (conectaExterno ? ";" : "(kP= ") + String(confValues.Kp, 1) +
                  (conectaExterno ? ";" : ", kI= ") + String(confValues.Ki, 1) +
                  (conectaExterno ? ";" : ", kD= ") + String(confValues.Kd, 1) +
                  (conectaExterno ? ";" : ")\tÂngulo: ") + String(angulo, 1) +
                  (conectaExterno ? ";" : ")\tSet point: ") + String(confValues.setpoint, 1) +
                  (conectaExterno ? ";" : ")\tAmostragem: ") + String(confValues.tempoAmostragem) +
                  (conectaExterno ? ";" : "\tTorque: ") + String(setpoint, 2) +
                  (conectaExterno ? ";" : ")\tDistância: ") + String(distancia, 1);
    if (millis() - contador > intervalo) {
      char data[50];
      info.toCharArray(data, (info.length() + 1));
      clienteMQTT.publish(topicoSaida, data);
      contador = millis();
    } else {
      Serial.println(info);
    }
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
    setpoint -= incrementoObjetivo;
  }
  if (setpoint < objetivo) {
    setpoint += incrementoObjetivo;
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
      case 'g':
        gravaEEPROM();
        break;
      case 'f':
        //Para frente
        objetivo = confValues.setpoint - 0.9;
        incrementoObjetivo = 0.03;
        break;
      case 'v':
        //Para trás
        objetivo = confValues.setpoint + 0.6;
        incrementoObjetivo = 0.01;
        break;
      case 'h':
        //Equilíbrio
        objetivo = confValues.setpoint;
        incrementoObjetivo = 0.1;
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
