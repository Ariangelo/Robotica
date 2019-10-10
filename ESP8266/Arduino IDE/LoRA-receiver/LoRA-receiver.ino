#include <ArduinoJson.h>
#include <SSD1306Wire.h>
#include <SPI.h>
#include <LoRa.h>

#define OLED_SDA   4
#define OLED_SCL  15
#define OLED_RST  16
#define ENDERECO_OLED 0x3C
#define TAMANHO       GEOMETRY_128_64

#define SPI_SCK   5
#define SPI_MISO  19
#define SPI_MOSI  27
#define CS_SS     18
#define RST       14
#define DI0       26
#define BAND      915E6

SSD1306Wire display(ENDERECO_OLED, OLED_SDA, OLED_SCL, TAMANHO); // SDA, SCL -> Configuracao do display SSD1306

void setup() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // seta GPIO16 LOW para resetar OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH); // enquanto OLED estiver ativo, GPIO16 deve estar em HIGH
  // Mostrar informacao no Display OLED
  display.init();
  display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
  display.setFont(ArialMT_Plain_24);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Conectando");
  display.display();
  // Mostrar informacao na porta Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println("Receptor LoRa");

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_SS);
  LoRa.setPins(CS_SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Falha ao conectar o LoRa");
    while (1);
  }
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(display.getWidth() / 2, 18, "Conectado");
  display.display();
}

void loop() {
  // Verifica se existe pacaote para ser processado
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String input = "";
    String infoDisplay;
    char strDisplay[30];
    Serial.print("Pacote recebido: ");
    // Leitura do pacote recebido
    while (LoRa.available()) {
      input += (char)LoRa.read();
    }
    Serial.println(input);
    // Criacao da instancia JSON para transporte de informacoes
    // Usado para criar o site https://arduinojson.org/v5/assistant/
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, input);
    // Verifica se a informação é valida
    if (error) {
      Serial.print("deserializeJson() falhou: ");
      Serial.println(error.c_str());
      return;
    } else {
      // Get the root object in the document
      JsonObject root = doc.as<JsonObject>();
      String hora = root["hora"];
      double temperatura = root["info"][0];
      double umidade = root["info"][1];
      sprintf(strDisplay, "%.1fºC  -  %.0f%%", temperatura, umidade);
      infoDisplay = strDisplay;
      Serial.print("Hora: ");
      Serial.println(hora);
      Serial.print("Temperatura: ");
      Serial.println(temperatura);
      Serial.print("Humidade: ");
      Serial.println(umidade);
      // Mostra RSSI do pacote
      Serial.print("com RSSI ");
      Serial.println(LoRa.packetRssi());
      // Mostra informacao atualizada da hora no display OLED
      display.clear();
      display.drawRect(0, 0, display.getWidth() - 1, display.getHeight() - 1);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setFont(ArialMT_Plain_16);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 20, hora);
      display.setFont(ArialMT_Plain_10);
      display.drawString(display.getWidth() / 2, display.getHeight() / 2 + (TAMANHO == GEOMETRY_128_64 ? 5 : 0), infoDisplay);
      display.display();
    }
  }
}
