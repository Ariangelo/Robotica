#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <LoRa.h>

#define SPI_SCK   5
#define SPI_MISO  19
#define SPI_MOSI  27
#define CS_SS     18
#define RST       14
#define DI0       26
#define BAND      915E6
#define TIME_ZONE    -3

const char* ssid  = "SSID";
const char* senha = "senha";

unsigned long contador;    // the debounce time; increase if the output flickers
unsigned long intervalo = 1000;     // Tempo em ms do intervalo a ser executado

WiFiUDP ntpUDP; // Cliente UDP (User Datagram Protocol) para o NTP (Network Time Protocol)
NTPClient horaCliente(ntpUDP, "pool.ntp.org", TIME_ZONE * 3600, 60000); // Configuracao do Cliente NTP

void setup() {
  // Conexao to Wi-Fi
  // Mostrar informacao na porta Serial
  Serial.println();
  Serial.begin(115200);
  Serial.println(ssid);
  WiFi.begin(ssid, senha);
  Serial.print("Conectando ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Mostra IP da conexao na porta Serial
  Serial.println();
  Serial.println("WiFi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Use este endereço para conectar ao ESP8266");
  Serial.println();
  Serial.println("Transmissor LoRa");

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, CS_SS);
  LoRa.setPins(CS_SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Falha ao conectar o LoRa");
    while (1);
  }
  horaCliente.begin();
  contador = millis();
}

void loop() {
  if (millis() - contador > intervalo) {
    // Criacao da instancia JSON para transporte de informacoes
    // Usado para criar o site https://arduinojson.org/v5/assistant/
    StaticJsonDocument<200> doc;
    JsonObject root = doc.to<JsonObject>();
    String strJson;
    horaCliente.update(); // Atualiza a hora no sistema utilizando o servidor NTP
    root["hora"] = horaCliente.getFormattedTime();
    JsonArray info = root.createNestedArray("info");
    info.add(29.75608);
    info.add(75.30203);
    serializeJson(root, strJson);
    Serial.print("Enviando pacote: ");
    Serial.println(strJson);
    // Envia via radio o pacote
    LoRa.beginPacket();
    LoRa.print(strJson);
    LoRa.endPacket();
    contador = millis();
  };
}
