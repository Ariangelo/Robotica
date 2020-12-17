// Sistema de calibracao do dispositivo MPU6050 desenvolvido por Ariangelo Hauer Dias <ariangelo@gmail.com> (13/05/2019)
// com base no feito por Luis Ródenas <luisrodenaslorda@gmail.com> Versão 1.1  (31/01/2104)
// com base na biblioteca I2Cdev e trabalho prévio de Jeff Rowberg <jeff@rowberg.net>
// Atualizações da biblioteca I2Cdev estão disponíveis em https://github.com/jrowberg/i2cdevlib

#include "I2Cdev.h"
#include "MPU6050_6aXis_MotionApps20.h"
#include "Wire.h"

MPU6050 mpu;
// controles MPU e variaveis de status
bool dmpPronto = false;
uint8_t statusDispositivo;
// controle operação de calibração
int tamanhoBuffer = 1000;   // Quantidade de leituras usadas para cálculo da média, quanto maior mais precisão, porém será mais lento (padrão: 1000)
int precisaoMPU = 8;        // Erro permitido, diminuir para obter mais precisão, porém poderá haver problema de convergencia para resultado (padrão: 8)
int precisaoGiro = 1;       // Erro de giro permitido, diminuir para obter mais precisão, porém poderá haver problema de convergencia para resultado (padrão: 1)
// Variáveis para procedimento de calibração
int16_t aX, aY, aZ;
int16_t gX, gY, gZ;
int axMedia, ayMedia, azMedia;
int gxMedia, gyMedia, gzMedia;
int axOffset , ayOffset, azOffset;
int gxOffset, gyOffset, gzOffset;
int fase = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("Inicializando dispositivos I2C...");
  mpu.initialize();
  Serial.println("Teste de conexão com o dispositivo MPU5060...");
  Serial.println(mpu.testConnection() ? "Dispositivo MPU6050 conectado com sucesso." : "A conexao com o MPU6050 falhou.");
  statusDispositivo = mpu.dmpInitialize();
  if (statusDispositivo == 0) {
    // ligar o DMP (Digital motion Processor)
    mpu.setDMPEnabled(true);
    dmpPronto = true;
    Serial.println("\nMPU6050 Procedimento de Calibração");
    Serial.println("\nO MPU6050 deve ser colocado na posição horizontal, com as escritas da placa de circuito impresso orientadas para cima.");
    Serial.println("Não toque no dispositivo até que seja mostrada a mensagem final.\n");
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
  switch (fase) {
    case 0:
      Serial.println("Lendo dados do sensor...");
      calculaMedia();
      fase++;
      delay(1000);
      break;
    case 1:
      Serial.println("Calculando valores de offsets...");
      calibracao();
      fase++;
      delay(1000);
      break;
    case 2:
      calculaMedia();
      Serial.println("\nCalibração finalizada!");
      Serial.println("\nSugestão para valores de offsets:");
      Serial.print("\n\tmpu.setXAccelOffset(");
      Serial.print(axOffset);
      Serial.println(");");
      Serial.print("\tmpu.setYAccelOffset(");
      Serial.print(ayOffset);
      Serial.println(");");
      Serial.print("\tmpu.setZAccelOffset(");
      Serial.print(azOffset);
      Serial.println(");");
      Serial.print("\tmpu.setXGyroOffset(");
      Serial.print(gxOffset);
      Serial.println(");");
      Serial.print("\tmpu.setYGyroOffset(");
      Serial.print(gyOffset);
      Serial.println(");");
      Serial.print("\tmpu.setZGyroOffset(");
      Serial.print(gzOffset);
      Serial.println(");");
      Serial.println("\nSe a calibração foi bem sucedida, copie os valores acima (Ctrl \"C\") e cole no seu projeto (Ctrl \"V\")");
      Serial.println("\nO procedimento de Calibração será repetido automaticamente em 10 segundos.");
      Serial.println();
      delay(10000); // Aguarda 10 segundos para iniciar novo processo
      fase = 0;
      break;
  }
}

void calculaMedia() {
  long i = 0;
  long axBuffer = 0, ayBuffer = 0, azBuffer = 0;
  long gxBuffer = 0, gyBuffer = 0, gzBuffer = 0;

  while (i < (tamanhoBuffer + 101)) {
    // Ler accel/gyro do MPU
    mpu.getMotion6(&aX, &aY, &aZ, &gX, &gY, &gZ);

    if (i > 100 && i <= (tamanhoBuffer + 100)) { // As primeiras 100 medidas serão descartadas
      axBuffer = axBuffer + aX;
      ayBuffer = ayBuffer + aY;
      azBuffer = azBuffer + aZ;
      gxBuffer = gxBuffer + gX;
      gyBuffer = gyBuffer + gY;
      gzBuffer = gzBuffer + gZ;
    }
    if (i == (tamanhoBuffer + 100)) {
      axMedia = axBuffer / tamanhoBuffer;
      ayMedia = ayBuffer / tamanhoBuffer;
      azMedia = azBuffer / tamanhoBuffer;
      gxMedia = gxBuffer / tamanhoBuffer;
      gyMedia = gyBuffer / tamanhoBuffer;
      gzMedia = gzBuffer / tamanhoBuffer;
      if (fase == 1) {
        Serial.print("{Buffer = (aX= ");
        Serial.print(axBuffer);
        Serial.print(" aY= ");
        Serial.print(ayBuffer);
        Serial.print(", Buffer aZ=");
        Serial.print(azBuffer);
        Serial.print(") - ");
      }
    }
    i++;
    delay(2); // Aguarda para evitar medidas repetidas
  }
}

void calibracao() {
  axOffset = -axMedia / 8;
  ayOffset = -ayMedia / 8;
  azOffset = (16384 - azMedia) / 8;

  gxOffset = -gxMedia / 4;
  gyOffset = -gyMedia / 4;
  gzOffset = -gzMedia / 4;

  int pronto = 0;
  while (pronto < 6) {
    mpu.setXAccelOffset(axOffset);
    mpu.setYAccelOffset(ayOffset);
    mpu.setZAccelOffset(azOffset);
    mpu.setXGyroOffset(gxOffset);
    mpu.setYGyroOffset(gyOffset);
    mpu.setZGyroOffset(gzOffset);

    calculaMedia();

    Serial.print("abs(Média) = (");
    Serial.print("aX= ");
    Serial.print(abs(axMedia));
    Serial.print(", aY= ");
    Serial.print(abs(ayMedia));
    Serial.print(", aZ= ");
    Serial.print(abs(16384 - axMedia));
    Serial.println(")}");

    if (abs(axMedia) <= precisaoMPU) {
      pronto++;
    }
    else {
      axOffset = axOffset - axMedia / precisaoMPU;
    }

    if (abs(ayMedia) <= precisaoMPU) {
      pronto++;
    } else {
      ayOffset = ayOffset - ayMedia / precisaoMPU;
    }

    if (abs(16384 - azMedia) <= precisaoMPU) {
      pronto++;
    } else {
      azOffset = azOffset + (16384 - azMedia) / precisaoMPU;
    }

    if (abs(gxMedia) <= precisaoGiro) {
      pronto++;
    } else {
      gxOffset = gxOffset - gxMedia / (precisaoGiro + 1);
    }

    if (abs(gyMedia) <= precisaoGiro) {
      pronto++;
    } else {
      gyOffset = gyOffset - gyMedia / (precisaoGiro + 1);
    }

    if (abs(gzMedia) <= precisaoGiro) {
      pronto++;
    } else {
      gzOffset = gzOffset - gzMedia / (precisaoGiro + 1);
    }
  }
}
