#include <Wire.h> 
#include "MPU9250.h"
#include "ModeloPatinhas.h" // Ficheiro gerado pelo Python

MPU9250 mpu;
Eloquent::ML::Port::RandomForest modelo; // Instância do modelo gerado

const int TAMANHO_JANELA = 100;
float leituras[TAMANHO_JANELA][6];
int indice = 0;

unsigned long lastSampleTime = 0;
const int SAMPLE_INTERVAL_MS = 20; // 50Hz

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);
  
  MPU9250Setting config;
  config.accel_fs_sel = ACCEL_FS_SEL::A2G;
  config.gyro_fs_sel = GYRO_FS_SEL::G250DPS;
  mpu.setup(0x68, config);
  
  Serial.println("Calibrando...");
  mpu.calibrateAccelGyro();
  Serial.println("Pronto.");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS) {
    lastSampleTime = currentTime;
    
    if (mpu.update()) {
      // Armazena as leituras brutas no vetor
      leituras[indice][0] = mpu.getAccX();
      leituras[indice][1] = mpu.getAccY();
      leituras[indice][2] = mpu.getAccZ();
      leituras[indice][3] = mpu.getGyroX();
      leituras[indice][4] = mpu.getGyroY();
      leituras[indice][5] = mpu.getGyroZ();
      indice++;
      
      // Quando atingir 100 leituras (2 segundos), processa os dados
      if (indice >= TAMANHO_JANELA) {
        classificarMovimento();
        indice = 0; // Reinicia a janela para a próxima classificação
      }
    }
  }
}

void classificarMovimento() {
  float features[12];
  
  // Calcula a média e o desvio padrão para cada um dos 6 eixos
  for (int eixo = 0; eixo < 6; eixo++) {
    float soma = 0;
    for (int i = 0; i < TAMANHO_JANELA; i++) {
      soma += leituras[i][eixo];
    }
    float media = soma / TAMANHO_JANELA;
    
    float somaVariancia = 0;
    for (int i = 0; i < TAMANHO_JANELA; i++) {
      somaVariancia += pow(leituras[i][eixo] - media, 2);
    }
    float desvioPadrao = sqrt(somaVariancia / TAMANHO_JANELA);
    
    // Os primeiros 6 valores são as médias, os 6 seguintes são os desvios
    features[eixo] = media;
    features[eixo + 6] = desvioPadrao;
  }
  
  // Envia as características extraídas para a IA e imprime o resultado
  Serial.print("Comportamento detetado: ");
  Serial.println(modelo.predictLabel(features));
}
