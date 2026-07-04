#include <WiFi.h>
#include <WiFiManager.h> 
#include <Wire.h> 
#include "MPU9250.h"
#include <U8g2lib.h>

// Configurações do Display
#define OLED_SCL 6
#define OLED_SDA 5
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
const int X_OFFSET = 30;
const int Y_OFFSET = 24;

WiFiServer server(80);
MPU9250 mpu;

// Variáveis de tempo (50Hz) obrigatórias para a IA
unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL_MS = 20;

// Callback do WiFiManager para atualizar o display
void configModeCallback(WiFiManager *myWiFiManager) {
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.print("Conecte na");
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
  u8g2.print("Rede Coleta");
  u8g2.sendBuffer();
  Serial.println("Modo de configuração ativado.");
}

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);

  // Inicializa o display
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.print("Buscando");
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
  u8g2.print("Wifi...");
  u8g2.sendBuffer();
  
  Serial.println("Iniciando WiFiManager...");
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  
  // Cria a rede caso não encontre o roteador conhecido
  if (!wifiManager.autoConnect("Patinhas-Coleta")) {
    Serial.println("Falha ao conectar. Reiniciando...");
    delay(3000);
    ESP.restart();
  }
  
  WiFi.setSleep(false); // Mantém estabilidade de energia
  
  Serial.println("\nWiFi conectado!");
  Serial.print("IP do ESP32: ");
  Serial.println(WiFi.localIP());

  // Mostra apenas "Wifi OK!"
  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.print("Wifi OK!");
  u8g2.sendBuffer();
  
  delay(2000); // Aguarda 2 segundos para leitura

  // Limpa a tela e mostra o IP dividido em duas linhas
  u8g2.clearBuffer();
  
  String metade1 = String(WiFi.localIP()[0]) + "." + String(WiFi.localIP()[1]) + ".";
  String metade2 = String(WiFi.localIP()[2]) + "." + String(WiFi.localIP()[3]);

  // Calcula a posição X ideal para centralizar cada linha (128 pixels é a largura total)
  int x1 = (128 - u8g2.getStrWidth(metade1.c_str())) / 2;
  int x2 = (128 - u8g2.getStrWidth(metade2.c_str())) / 2;

  // Evita que a coordenada seja negativa
  if (x1 < 0) x1 = 0;
  if (x2 < 0) x2 = 0;

  u8g2.setCursor(x1, Y_OFFSET + 20); 
  u8g2.print(metade1.c_str());

  u8g2.setCursor(x2, Y_OFFSET + 40); 
  u8g2.print(metade2.c_str());

  u8g2.sendBuffer();

  server.begin();
  
  MPU9250Setting config;
  config.accel_fs_sel = ACCEL_FS_SEL::A2G;
  config.gyro_fs_sel = GYRO_FS_SEL::G250DPS;
  mpu.setup(0x68, config);
  
  Serial.println("Calibrando sensores...");
  mpu.calibrateAccelGyro();
  Serial.println("Sistema Pronto. Aguardando Python...");
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente Python conectado!");
    
    // Envia o cabeçalho CSV 
    client.println("accX,accY,accZ,gyroX,gyroY,gyroZ");
    
    while (client.connected()) {
      unsigned long currentTime = millis();
      
      // Controle rígido de tempo (50Hz)
      if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
        lastSampleTime = currentTime;
        
        if (mpu.update()) {
          // Envia os dados separados por vírgula para o Python
          client.print(mpu.getAccX()); client.print(",");
          client.print(mpu.getAccY()); client.print(",");
          client.print(mpu.getAccZ()); client.print(",");
          client.print(mpu.getGyroX()); client.print(",");
          client.print(mpu.getGyroY()); client.print(",");
          client.println(mpu.getGyroZ());
        }
      }
    }
    client.stop();
    Serial.println("Cliente desconectado.");
  }
}