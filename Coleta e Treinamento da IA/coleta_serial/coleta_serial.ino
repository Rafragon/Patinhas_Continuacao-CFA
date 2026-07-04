#include <Wire.h> 
#include "MPU9250.h"
#include <U8g2lib.h>

// ========= CONFIGURAÇÕES DO SENSOR =========
MPU9250 mpu; 

// ========= CONFIGURAÇÕES INICIAIS DO DISPLAY =========
#define OLED_SCL 6  // Pino SCL
#define OLED_SDA 5  // Pino SDA
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
const int X_OFFSET = 30;
const int Y_OFFSET = 24;

// ========= CONTROLE DE AMOSTRAGEM (DATASET) =========
unsigned long lastSampleTime = 0;
const int SAMPLE_RATE_HZ = 50; // 50 amostras por segundo
const int SAMPLE_INTERVAL_MS = 1000 / SAMPLE_RATE_HZ; // 20 milissegundos

void setup() {
  Serial.begin(115200);

  // CONFIGURAÇÃO DOS PINOS DO ESP32-C3
  Wire.begin(5, 6);

  // Inicializa display para dar feedback visual
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.setCursor(X_OFFSET - 10, Y_OFFSET + 20);
  u8g2.print("Modo Coleta");
  u8g2.sendBuffer();

  // Calibração do MPU
  MPU9250Setting config;
  config.accel_fs_sel = ACCEL_FS_SEL::A2G;
  config.gyro_fs_sel  = GYRO_FS_SEL::G250DPS;
  mpu.setup(0x68, config); 
  
  // Atraso de 3 segundos para dar tempo de abrir o monitor serial
  delay(3000); 

  Serial.println("Calibrando sensor (deixe parado)...");
  mpu.calibrateAccelGyro();
  mpu.setAccBias(mpu.getAccBiasX(), mpu.getAccBiasY(), mpu.getAccBiasZ());
  mpu.setGyroBias(mpu.getGyroBiasX(), mpu.getGyroBiasY(), mpu.getGyroBiasZ());
  
  Serial.println("Calibracao concluida!");
  delay(1000);
  
  // Imprime o cabeçalho para o nosso arquivo CSV
  Serial.println("accX,accY,accZ,gyroX,gyroY,gyroZ");
}

void loop() {
  unsigned long currentTime = millis();

  // Só faz a leitura exatamente a cada 20ms (para garantir 50Hz perfeitos)
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = currentTime;

    if (mpu.update()) {
      // Imprime os valores brutos separados por vírgula
      Serial.print(mpu.getAccX()); Serial.print(",");
      Serial.print(mpu.getAccY()); Serial.print(",");
      Serial.print(mpu.getAccZ()); Serial.print(",");
      Serial.print(mpu.getGyroX()); Serial.print(",");
      Serial.print(mpu.getGyroY()); Serial.print(",");
      Serial.println(mpu.getGyroZ());
    }
  }
}