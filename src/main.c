#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>

#define SS_PIN  5  // Define o pino SS (Slave Select) do MFRC522
#define RST_PIN 22  // Define o pino de Reset do MFRC522
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "your-SSID";  // Nome da rede Wi-Fi
const char* password = "your-PASSWORD";  // Senha da rede Wi-Fi
const char* serverBaseUrl = "http://your-server.com/api";  // URL base do servidor

// Variável global para armazenar o cookie de autenticação
String authCookie = "";

WiFiClient client;

// Definindo o pino do buzzer
#define BUZZER_PIN 12  // Pino conectado ao buzzer

void setup() {
  // Inicializando comunicação serial
  Serial.begin(115200);

  // Inicializando o leitor RFID
  SPI.begin();  // Inicia a comunicação SPI
  rfid.PCD_Init();  // Inicializa o MFRC522
  
  // Configuração do buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");

  // Realizar o login
  login();
}

void loop() {
  // Verificando se há um cartão RFID presente
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardCode = getCardCode();
    Serial.print("Código do Cartão RFID: ");
    Serial.println(cardCode);

    // Verificar a cor do cartão no servidor
    String cardColor = getCardColor(cardCode);
    Serial.print("Cor do Cartão: ");
    Serial.println(cardColor);

    // Determinar a direção (IN ou OUT)
    RegistrationDirection direction = (cardColor == "RED") ? RegistrationDirection::IN : RegistrationDirection::OUT;

    // Emitir um som diferente no buzzer dependendo da cor
    playBuzzerSound(cardColor);

    // Obter o userId a partir do código do cartão (a lógica pode variar dependendo de como está implementado no backend)
    long userId = getUserIdByCardCode(cardCode);

    // Registrar o evento (IN ou OUT)
    registerEvent(direction, userId);

    delay(1000); // Atraso entre leituras de cartões
  }
}

// Função para fazer login e armazenar o cookie
void login() {
  HTTPClient http;
  String jsonBody = "{\"username\":\"root\",\"password\":\"root\"}";
  
  http.begin(client, String(serverBaseUrl) + "/authentication/login");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(jsonBody);
  
  if (httpCode > 0) {
    String cookie = http.header("Set-Cookie");
    Serial.print("Cookie Recebido: ");
    Serial.println(cookie);
    authCookie = cookie;  // Armazenando o cookie
  } else {
    Serial.println("Erro ao fazer login.");
  }
  http.end();
}

// Função para obter o código do cartão RFID
String getCardCode() {
  String cardCode = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardCode += String(rfid.uid.uidByte[i], HEX);
  }
  return cardCode;
}

// Função para verificar a cor do cartão via servidor
String getCardColor(String cardCode) {
  HTTPClient http;
  String url = String(serverBaseUrl) + "/carte/" + cardCode;

  http.begin(client, url);
  http.addHeader("Cookie", authCookie);  // Envia o cookie de autenticação
  int httpCode = http.GET();
  
  String cardColor = "";
  if (httpCode > 0) {
    String response = http.getString();
    // Aqui você pode parsear a resposta para extrair a cor (depende do formato da resposta)
    // Exemplo:
    if (response == "RED" || response == "YELLOW" || response == "GREEN") {
      cardColor = response;
    }
  } else {
    Serial.println("Erro ao verificar a cor do cartão.");
  }
  http.end();
  return cardColor;
}

// Função para obter o userId a partir do código do cartão (implementação fictícia)
long getUserIdByCardCode(String cardCode) {
  // Aqui você pode consultar o backend para obter o userId correspondente ao código do cartão
  // Para o exemplo, vamos assumir que o userId é fixo para cada cor
  if (cardCode == "123456") return 1;  // Exemplo de userId
  return 0;  // Caso não encontre
}

// Função para registrar o evento de entrada/saída
void registerEvent(RegistrationDirection direction, long userId) {
  HTTPClient http;
  String jsonBody = "{\"direction\":\"" + String(direction == RegistrationDirection::IN ? "IN" : "OUT") + "\","
                    "\"userId\":" + String(userId) + ","
                    "\"createdAt\":\"" + String(millis()) + "\"}";  // Usando millis() como timestamp

  http.begin(client, String(serverBaseUrl) + "/registration");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Cookie", authCookie);  // Envia o cookie de autenticação
  int httpCode = http.POST(jsonBody);
  
  if (httpCode > 0) {
    Serial.println("Evento registrado com sucesso.");
  } else {
    Serial.println("Erro ao registrar o evento.");
  }
  http.end();
}

// Função para tocar som no buzzer dependendo da cor do cartão
void playBuzzerSound(String cardColor) {
  if (cardColor == "RED") {
    tone(BUZZER_PIN, 1000, 500);  // Som para "pode entrar" (tom médio)
  } else {
    tone(BUZZER_PIN, 2000, 500);  // Som para "não pode entrar" (tom alto)
  }
}
