#include <ESP8266WiFi.h>

const char* ssid = "seu ssid";
const char* senha = "sua senha";
const String servidor = "192.168.43.49"; // seu servidor
String path;

int pinoBotao = 0;
int botao = 0;         // variable for reading the pushbotao status

void setup () {
  pinMode(pinoBotao, INPUT);

  Serial.begin(115200);
  // Conexao to Wi-Fi
  Serial.print("Conectando ");
  Serial.println(ssid);
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Mostra IP do servidor
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Use este endereço para conectar ao ESP8266");
  Serial.println();
}

void loop() {
  WiFiClient client;
  //Le o valor da porta analogica 
  int leitura = analogRead(pinoBotao);
  //Testa o valor da resistencia do circuito -> leitura porta analogica
  //Os valores adotados dependem dos valores dos resistores utilileituraados
  //Recomenda-se adotar uma faixa de variacao dos valores obtidos
  if (leitura > 200 && leitura < 220) botao = 1; //btnEsquerda
  else if (leitura > 255 && leitura < 275) botao = 2; //btnCima
  else if (leitura > 345 && leitura < 365) botao = 3; //btnDireita
  else if (leitura > 525 && leitura < 545) botao = 4; //btnBaixo
  else botao = 0;

  if (!client.connect(servidor, 80)) {
    Serial.println("Conexao falhou!");
    return;
  }
  //Verifica qual a acao a ser executada em funcao do botao pressionado
  switch (botao) {
    case 1:
      path = "btnEsquerda";
      break;
    case 2:
      path = "btnCima";
      break;
    case 3:
      path = "btnDireita";
      break;
    case 4:
      path = "btnBaixo";
      break;
    default:
      path = "btnHome";
  }
  // Envia a requisicao ao servidor com a acao desejada -> path
  client.print(String("GET /") + path + " HTTP/1.1\r\n" +
               "Host: " + servidor + "\r\n" +
               "Connection: close\r\n" +
               "\r\n");
  client.stop();
  delay(100); //Tempo para não sobrecarregar o servidor
}

