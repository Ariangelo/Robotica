#include <ESP8266WiFi.h>

#define PIN_FRENTE    12
#define PIN_TRAZ      13
#define PIN_ESQUERDA  14
#define PIN_DIREITA   15

const char* ssid = "ssid";
const char* senha = "senha";

int frenteLigado = LOW;
int trazLigado = LOW;
int esquerdaLigado = LOW;
int direitaLigado = LOW;

// Porta web server = 80
WiFiServer server(80);

void setup() {
  pinMode(PIN_FRENTE, OUTPUT);
  pinMode(PIN_TRAZ, OUTPUT);
  pinMode(PIN_ESQUERDA, OUTPUT);
  pinMode(PIN_DIREITA, OUTPUT);

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

  server.begin();
}

void loop() {
  WiFiClient client = server.available();   // Aguarda requisicoes de clientes
  if (client) {                             // Se cliente conectado,
    String cabec = ""; // Armazenamento HTTP request
    boolean linhaAtualVazia = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        cabec += c; // armazena conteudo do cabecalho para processamento
        // se é o final da linha e a linha está em branco,
        // significa que a solicitação http terminou,
        // desta forma e possível enviar uma resposta http
        if (c == '\n' && linhaAtualVazia) {
          // processa o comando GET enviado do cliente
          processaGET(cabec);
          // envia um cabecalho de resposta http padrao
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // a conexao será fechada após a conclusão da resposta
          client.println();
          // Pagina HTML
          client.println("<!doctype html>");
          client.println("<html>");
          client.println("   <head>");
          client.println("      <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.0/jquery.min.js\"></script>");
          client.println("      <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.8.1/css/fontawesome.min.css\">");
          client.println("      <style>");
          client.println("         .content {");
          client.println("         max-width: 280px;");
          client.println("         margin: auto;");
          client.println("         }");
          client.println("         .grid {");
          client.println("         display: grid;");
          client.println("         border-radius: 1em;");
          client.println("         grid-gap: 10px;");
          client.println("         grid-template-columns: 80px 80px 80px;");
          client.println("         background-color: LAVENDER;");
          client.println("         padding: 10px;");
          client.println("         }");
          client.println("         .grid-item {");
          client.println("         border: none;");
          client.println("         padding: 10px;");
          client.println("         background-color: POWDERBLUE;");
          client.println("         color: White;");
          client.println("         border-radius: 10px;");
          client.println("         font-size: 200%;");
          client.println("         text-align: center;");
          client.println("        }");
          client.println("      </style>");
          client.println("   </head>");
          client.println("   <body>");
          client.println("      <div class=\"content\">");
          client.println("         <h3 align=\"center\">Click na seta correspondente ao movimento desejado, para parar click Home.</h3>");
          client.println("         <div class=\"grid\">");
          client.println("            <div id=\"btnCimaEsq\" class=\"grid-item\"><span class=\"fa\" style=\"transform: rotate(-45deg);\">&#xf0aa;</span></div>");
          client.println("            <div id=\"btnCima\" class=\"grid-item\"><span class=\"fa\">&#xf0aa;</span></div>");
          client.println("            <div id=\"btnCimaDir\" class=\"grid-item\"><span class=\"fa\" style=\"transform: rotate(45deg);\">&#xf0aa;</span></div>");
          client.println("            <div id=\"btnEsquerda\" class=\"grid-item\"><span class=\"fa\">&#xf0a8;</span></div>");
          client.println("            <div id=\"btnHome\" class=\"grid-item\"><span class=\"fa\">&#xf015;</span></div>");
          client.println("            <div id=\"btnDireita\" class=\"grid-item\"><span class=\"fa\">&#xf0a9;</span></div>");
          client.println("            <div id=\"btnBaixoEsq\" class=\"grid-item\"><span class=\"fa\" style=\"transform: rotate(45deg);\">&#xf0ab;</span></div>");
          client.println("            <div id=\"btnBaixo\" class=\"grid-item\"><span class=\"fa\">&#xf0ab;</span></div>");
          client.println("            <div id=\"btnBaixoDir\" class=\"grid-item\"><span class=\"fa\" style=\"transform: rotate(-45deg);\">&#xf0ab;</span></div>");
          client.println("         </div>");
          client.println("      </div>");
          client.println("      <script>");
          client.println("         $(document).ready(function(){");
          client.println("             var button = $(\".grid-item\");");
          client.println("             button.on(\"touchstart mousedown\", function(){");
          client.println("                 $(this).css(\"backgroundColor\", \"Blue\");");
          client.println("                 window.location.pathname = \"/\" + $(this)[0].id;");
          client.println("             });");
          client.println("             button.on(\"touchleave touchend mouseup\", function(){");
          client.println("                 $(this).css(\"backgroundColor\", \"POWDERBLUE\");");
          client.println("                 window.location.pathname = \"/btnHome\";");
          client.println("             });");
          client.println("         });");
          client.println("      </script>");
          client.println("  </body>");
          client.println("</html>");
          // Envia a HTTP response
          client.println();
          break;
        }
        if (c == '\n') {  // inicio de uma nova linha
          linhaAtualVazia = true;
        } else if (c != '\r') {  // existe pelo menos um caracter na linha atual
          linhaAtualVazia = false;
        }
      }
    }
    // um atraso para o web browser processar a resposta
    delay(10);
    // close the connection:
    // Fecha conexao
    client.stop();
  }
}

void processaGET(String c) {
  if (c.indexOf("GET /") >= 0) {
    String linha = c.substring(c.indexOf("GET /") + 5);
    int ind = 0;
    String info = "";
    while (linha[ind] != '\n') {
      info += linha[ind];
      ind++;
    }
    String acao = info.substring(0, info.indexOf(" HTTP/1.1"));
    if (acao == "btnCimaEsq") {
      frenteLigado = HIGH;
      esquerdaLigado = HIGH;
    }
    else if (acao == "btnCima") {
      frenteLigado = HIGH;
    }
    else if (acao == "btnCimaDir") {
      frenteLigado = HIGH;
      direitaLigado = HIGH;
    }
    else if (acao == "btnEsquerda") {
      esquerdaLigado = HIGH;
    }
    else if (acao == "btnHome") {
      frenteLigado = LOW;
      trazLigado = LOW;
      esquerdaLigado = LOW;
      direitaLigado = LOW;
    }
    else if (acao == "btnDireita") {
      direitaLigado = HIGH;
    }
    if (acao == "btnBaixoEsq") {
      trazLigado = HIGH;
      esquerdaLigado = HIGH;
    }
    else if (acao == "btnBaixo") {
      trazLigado = HIGH;
    }
    else if (acao == "btnBaixoDir") {
      trazLigado = HIGH;
      direitaLigado = HIGH;
    }

  }

  //Atualiza as saidas
  digitalWrite(PIN_FRENTE, frenteLigado);
  digitalWrite(PIN_TRAZ, trazLigado);
  digitalWrite(PIN_ESQUERDA, esquerdaLigado);
  digitalWrite(PIN_DIREITA, direitaLigado);
}
