#include <Wire.h> 
#include "MPU9250.h"
#include <U8g2lib.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include "ModeloPatinhas.h" 
#include <WiFiManager.h>

// ========= CONFIGURAÇÕES DE REDE =========
WiFiUDP udp;
const int broadcastPort = 50000;

// ========= VARIÁVEIS DO SERVIDOR =========
bool serverEncontrado = false;
char serverIP[32] = ""; 
const int serverPort = 8080;

// ========= VARIÁVEIS DE ENVIO =========
unsigned long lastPost = 0;

// ========= CONFIGURAÇÕES DO SENSOR =========
MPU9250 mpu;
float rawAccX, rawAccY, rawAccZ;
float A; 
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL_MS = 20; // 50Hz obrigatório para a IA

// ========= CONFIGURAÇÕES DO DISPLAY =========
#define OLED_SCL 6
#define OLED_SDA 5
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
const int X_OFFSET = 30;
const int Y_OFFSET = 24;
unsigned long ultimoUpdateDisplay = 0;
const int INTERVALO_DISPLAY = 100;

// ========= VARIÁVEIS DA IA =========
Eloquent::ML::Port::RandomForest modelo;
const int TAMANHO_JANELA = 100;
float leiturasIA[TAMANHO_JANELA][6];
int indiceIA = 0;

// ========= ALGORITMO DE PASSOS =========
const int TAMANHO_FILTRO = 3;
float leiturasFiltro[TAMANHO_FILTRO];
int indiceFiltro = 0;
float A_filtrado;

bool esperandoPico = true;
float impactoDoPasso = 1.2;
float impactoDoRepouso = 0.95;
unsigned long momentoDoPassoAnterior = 0;
int tempoEntrePassos = 100;
int passos = 0;

float gyroMagnitude;
float limiarDeRotacao = 100.0;
unsigned long ultimoMomentoDeRotacao = 0;  
const int COOLDOWN_APOS_ROTACAO = 500;
unsigned long momentoDoPico = 0;   
const int tempoMaximoParaRepouso = 400;

void configModeCallback(WiFiManager *myWiFiManager) {
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.print("Conecte na");
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
  u8g2.print("Rede Patinhas");
  u8g2.sendBuffer();
  Serial.println("Modo de configuração ativado.");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_spleen12x24_me);

  // Aviso inicial no display
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.print("Buscando");
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
  u8g2.print("Wifi...");
  u8g2.sendBuffer();

  WiFiManager wifiManager;
  
  // Define a função que atualiza o display se o ESP32 entrar em modo AP
  wifiManager.setAPCallback(configModeCallback);

  // Tenta conectar à última rede. Se falhar, cria a rede "Patinhas-Config" sem senha.
  // O código pausa aqui até que a configuração seja feita pelo smartphone.
  if (!wifiManager.autoConnect("Patinhas-Config")) {
    Serial.println("Falha ao conectar. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  // Configuração pós-conexão
  WiFi.setSleep(false); // Mantém a estabilidade da bateria durante transmissão

  Serial.println("\nConectado!");
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.print("Wifi OK!");
  u8g2.sendBuffer();
  delay(2000);

  MPU9250Setting config;
  config.accel_fs_sel = ACCEL_FS_SEL::A2G;
  config.gyro_fs_sel  = GYRO_FS_SEL::G250DPS;
  mpu.setup(0x68, config);
  
  Serial.println("Calibrando sensor...");
  mpu.calibrateAccelGyro();
  mpu.setAccBias(mpu.getAccBiasX(), mpu.getAccBiasY(), mpu.getAccBiasZ());
  mpu.setGyroBias(mpu.getGyroBiasX(), mpu.getGyroBiasY(), mpu.getGyroBiasZ());

  for (int i = 0; i < TAMANHO_FILTRO; i++) {
    leiturasFiltro[i] = 1.0;
  }

  udp.begin(broadcastPort);
  broadcastProcuraServidor();
}

void broadcastProcuraServidor(){
  Serial.println("Procurando servidor na rede...");
  while (!serverEncontrado) {
    udp.beginPacket(IPAddress(255,255,255,255), broadcastPort);
    udp.print("DISCOVER_SERVER");
    udp.endPacket();

    int len = udp.parsePacket();
    if (len > 0) {
      char buf[64];
      udp.read(buf, len);
      buf[len] = 0;
      if (strncmp(buf, "SERVER_IP:", 10) == 0) {
        strncpy(serverIP, buf + 10, sizeof(serverIP));
        serverEncontrado = true;
        Serial.print("Servidor encontrado: ");
        Serial.println(serverIP);
      }
    }
    delay(1000);
  }
}

void sendPost(int mensagem) {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    char URL[100];
    sprintf(URL, "http://%s:%d/", serverIP, serverPort);
    http.begin(URL);
    http.addHeader("Content-Type", "text/plain");
    
    int httpResponseCode = http.POST(String(mensagem));
    http.end();
  }
}

void sendEstadoPost(String estado) {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    char URL[100];
    sprintf(URL, "http://%s:%d/estado", serverIP, serverPort); // Endpoint ajustado para diferenciar estados
    http.begin(URL);
    http.addHeader("Content-Type", "text/plain");
    
    int httpResponseCode = http.POST(estado);
    http.end();
  }
}

void processarIA() {
  float features[12];
  
  for (int eixo = 0; eixo < 6; eixo++) {
    float soma = 0;
    for (int i = 0; i < TAMANHO_JANELA; i++) {
      soma += leiturasIA[i][eixo];
    }
    float media = soma / TAMANHO_JANELA;
    
    float somaVariancia = 0;
    for (int i = 0; i < TAMANHO_JANELA; i++) {
      somaVariancia += pow(leiturasIA[i][eixo] - media, 2);
    }
    float desvioPadrao = sqrt(somaVariancia / TAMANHO_JANELA);
    
    features[eixo] = media;
    features[eixo + 6] = desvioPadrao;
  }
  
  String estadoAtual = String(modelo.predictLabel(features));
  Serial.print("Estado detetado: ");
  Serial.println(estadoAtual);
  sendEstadoPost(estadoAtual);
}

void loop() {
  unsigned long currentTime = millis();

  // Executa a amostragem a 50Hz exatos
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = currentTime;
    
    if (mpu.update()) {
      rawAccX = mpu.getAccX();
      rawAccY = mpu.getAccY();
      rawAccZ = mpu.getAccZ();
      A = sqrt(pow(rawAccX, 2) + pow(rawAccY, 2) + pow(rawAccZ, 2));

      float gyroX = mpu.getGyroX();
      float gyroY = mpu.getGyroY();
      float gyroZ = mpu.getGyroZ();
      gyroMagnitude = sqrt(pow(gyroX, 2) + pow(gyroY, 2) + pow(gyroZ, 2));

      // Armazenamento para a IA
      leiturasIA[indiceIA][0] = rawAccX;
      leiturasIA[indiceIA][1] = rawAccY;
      leiturasIA[indiceIA][2] = rawAccZ;
      leiturasIA[indiceIA][3] = gyroX;
      leiturasIA[indiceIA][4] = gyroY;
      leiturasIA[indiceIA][5] = gyroZ;
      indiceIA++;

      if (indiceIA >= TAMANHO_JANELA) {
        processarIA();
        indiceIA = 0;
      }

      if (gyroMagnitude > limiarDeRotacao) {
        ultimoMomentoDeRotacao = currentTime;
      }

      // Filtro Média Móvel (Passos)
      float soma = 0;
      leiturasFiltro[indiceFiltro] = A;
      indiceFiltro = (indiceFiltro + 1) % TAMANHO_FILTRO;
      for (int i = 0; i < TAMANHO_FILTRO; i++) {
        soma += leiturasFiltro[i];
      }
      A_filtrado = soma / TAMANHO_FILTRO;

      // Máquina de Estados (Passos)
      if (esperandoPico && (A_filtrado > impactoDoPasso) && (gyroMagnitude < limiarDeRotacao) && ((currentTime - ultimoMomentoDeRotacao) > COOLDOWN_APOS_ROTACAO)) {    
        esperandoPico = false;
        momentoDoPico = currentTime;
      }
      else if (!esperandoPico) {
        if (A_filtrado < impactoDoRepouso) { 
          if ((currentTime - momentoDoPassoAnterior) > tempoEntrePassos) {   
            passos++;
            sendPost(1);
            
            Serial.print("Passo Detetado! Total: ");
            Serial.println(passos);
            momentoDoPassoAnterior = currentTime;
            
            u8g2.clearBuffer();
            u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
            u8g2.setFont(u8g2_font_spleen12x24_me);
            u8g2.print(passos);
            u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
            u8g2.setFont(u8g2_font_bauhaus2015_tr);
            u8g2.print("Passos");
          }
          esperandoPico = true;
        }
        else if ((currentTime - momentoDoPico) > tempoMaximoParaRepouso) {
          esperandoPico = true;
        }
      }
    }
  }

  // Atualização do ecrã
  if (millis() - ultimoUpdateDisplay > INTERVALO_DISPLAY) {
    u8g2.sendBuffer();
    ultimoUpdateDisplay = millis();
  }
}