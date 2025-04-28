#include <WiFi.h>
#include <HTTPClient.h>
#include <MFRC522.h>

#define SS_PIN  5  // Define o pino SS (Slave Select) do MFRC522
#define RST_PIN 22  // Define o pino de Reset do MFRC522
MFRC522 rfid(SS_PIN, RST_PIN);

const char* ssid = "Familia Dias";
const char* password = "Leao.2006";
const char* serverBaseUrl = "http://localhost:8080/api/registration";

String authCookie = "";

WiFiClient client;

#define BUZZER_PIN 12  // Pino conectado ao buzzer

enum RegistrationDirection {
  IN,
  OUT 
};

void setup() {
  Serial.begin(115200);

  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }
  Serial.println("Conectado ao Wi-Fi");

  login();
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String cardCode = getCardCode();
    Serial.print("Código do Cartão RFID: ");
    Serial.println(cardCode);

    String cardColor = getCardColor(cardCode);
    Serial.print("Cor do Cartão: ");
    Serial.println(cardColor);
    
    RegistrationDirection direction = (cardColor == "RED") ? IN : OUT;

    playBuzzerSound(cardColor);

    long userId = getUserIdByCardCode(cardCode);
    
    registerEvent(direction, userId);

    delay(1000); 
}

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
    authCookie = cookie;
  } else {
    Serial.println("Erro ao fazer login.");
  }
  http.end();
}

String getCardCode() {
  String cardCode = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    cardCode += String(rfid.uid.uidByte[i], HEX);
  }
  return cardCode;
}

String getCardColor(String cardCode) {
  HTTPClient http;
  String url = String(serverBaseUrl) + "/carte/" + cardCode;

  http.begin(client, url);
  http.addHeader("Cookie", authCookie);
  int httpCode = http.GET();
  
  String cardColor = "";
  if (httpCode > 0) {
    String response = http.getString();
    if (response == "RED" || response == "YELLOW" || response == "GREEN") {
      cardColor = response;
    }
  } else {
    Serial.println("Erro ao verificar a cor do cartão.");
  }
  http.end();
  return cardColor;
}

long getUserIdByCardCode(String cardCode) {
  if (cardCode == "123456") return 1;
  return 0;
}

void registerEvent(RegistrationDirection direction, long userId) {
  HTTPClient http;
  String jsonBody = "{\"direction\":\"" + String(direction == IN ? "IN" : "OUT") + "\","
                    "\"userId\":" + String(userId) + ","
                    "\"createdAt\":\"" + String(millis()) + "\"}";  
  http.begin(client, String(serverBaseUrl) + "/registration");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Cookie", authCookie); 
  int httpCode = http.POST(jsonBody);
  
  if (httpCode > 0) {
    Serial.println("Evento registrado com sucesso.");
  } else {
    Serial.println("Erro ao registrar o evento.");
  }
  http.end();
}

void playBuzzerSound(String cardColor) {
  if (cardColor == "RED") {
    tone(BUZZER_PIN, 1000, 500);
  } else {
    tone(BUZZER_PIN, 2000, 500);
  }
}
