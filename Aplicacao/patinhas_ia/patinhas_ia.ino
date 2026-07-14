#include <Wire.h> 
#include "MPU9250.h"
#include <U8g2lib.h>
#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include "ModeloPatinhas.h" 
#include <WiFiManager.h>

// Bibliotecas do FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

// ========= SINCRONIZAÇÃO FREERTOS =========
SemaphoreHandle_t mutexI2C;
QueueHandle_t filaPassos;
TaskHandle_t handleTaskIA;

// ========= CONFIGURAÇÕES DE REDE =========
WiFiUDP udp;
const int broadcastPort = 50000;

// ========= VARIÁVEIS DO SERVIDOR =========
bool serverEncontrado = false;
char serverIP[32] = ""; 
const int serverPort = 8080;

// ========= CONFIGURAÇÕES DO SENSOR =========
MPU9250 mpu;
const int SAMPLE_INTERVAL_MS = 20;

// ========= CONFIGURAÇÕES DO DISPLAY =========
#define OLED_SCL 6
#define OLED_SDA 5
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
const int X_OFFSET = 30;
const int Y_OFFSET = 24;
const int INTERVALO_DISPLAY = 100;

// ========= VARIÁVEIS DA IA (DOUBLE BUFFERING) =========
Eloquent::ML::Port::RandomForest modelo;
const int TAMANHO_JANELA = 100;
float buffersIA[2][TAMANHO_JANELA][6];
volatile int bufferPreenchimento = 0;
volatile int bufferProcessamento = 0;
volatile int indiceIA = 0;

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
volatile int totalPassos = 0;

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
    http.POST(String(mensagem));
    http.end();
  }
}

void sendEstadoPost(String estado) {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    char URL[100];
    sprintf(URL, "http://%s:%d/estado", serverIP, serverPort);
    http.begin(URL);
    http.addHeader("Content-Type", "text/plain");
    http.POST(estado);
    http.end();
  }
}

// ========= TASK 1: SENSOR E PASSOS (PRIORIDADE ALTA) =========
void taskSensor(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(SAMPLE_INTERVAL_MS);

  for(;;) {
    bool updateSucesso = false;
    float rawAccX, rawAccY, rawAccZ, gyroX, gyroY, gyroZ;
    
    // Bloqueia barramento I2C
    if (xSemaphoreTake(mutexI2C, portMAX_DELAY) == pdTRUE) {
      updateSucesso = mpu.update();
      if (updateSucesso) {
        rawAccX = mpu.getAccX();
        rawAccY = mpu.getAccY();
        rawAccZ = mpu.getAccZ();
        gyroX = mpu.getGyroX();
        gyroY = mpu.getGyroY();
        gyroZ = mpu.getGyroZ();
      }
      xSemaphoreGive(mutexI2C); // Libera barramento imediatamente
    }

    if (updateSucesso) {
      unsigned long currentTime = millis();
      float A = sqrt(pow(rawAccX, 2) + pow(rawAccY, 2) + pow(rawAccZ, 2));
      gyroMagnitude = sqrt(pow(gyroX, 2) + pow(gyroY, 2) + pow(gyroZ, 2));

      // Preenchimento do Double Buffer
      buffersIA[bufferPreenchimento][indiceIA][0] = rawAccX;
      buffersIA[bufferPreenchimento][indiceIA][1] = rawAccY;
      buffersIA[bufferPreenchimento][indiceIA][2] = rawAccZ;
      buffersIA[bufferPreenchimento][indiceIA][3] = gyroX;
      buffersIA[bufferPreenchimento][indiceIA][4] = gyroY;
      buffersIA[bufferPreenchimento][indiceIA][5] = gyroZ;
      indiceIA++;

      if (indiceIA >= TAMANHO_JANELA) {
        bufferProcessamento = bufferPreenchimento;
        bufferPreenchimento = (bufferPreenchimento == 0) ? 1 : 0; // Troca de matriz
        indiceIA = 0;
        xTaskNotifyGive(handleTaskIA); // Avisa a Task de Rede para processar
      }

      if (gyroMagnitude > limiarDeRotacao) {
        ultimoMomentoDeRotacao = currentTime;
      }

      // Filtro e Máquina de Estados
      float soma = 0;
      leiturasFiltro[indiceFiltro] = A;
      indiceFiltro = (indiceFiltro + 1) % TAMANHO_FILTRO;
      for (int i = 0; i < TAMANHO_FILTRO; i++) {
        soma += leiturasFiltro[i];
      }
      A_filtrado = soma / TAMANHO_FILTRO;

      if (esperandoPico && (A_filtrado > impactoDoPasso) && (gyroMagnitude < limiarDeRotacao) && ((currentTime - ultimoMomentoDeRotacao) > COOLDOWN_APOS_ROTACAO)) {    
        esperandoPico = false;
        momentoDoPico = currentTime;
      }
      else if (!esperandoPico) {
        if (A_filtrado < impactoDoRepouso) { 
          if ((currentTime - momentoDoPassoAnterior) > tempoEntrePassos) {   
            totalPassos++;
            int msg = 1;
            xQueueSend(filaPassos, &msg, 0); // Envia sinal de passo para Task de Rede
            momentoDoPassoAnterior = currentTime;
          }
          esperandoPico = true;
        }
        else if ((currentTime - momentoDoPico) > tempoMaximoParaRepouso) {
          esperandoPico = true;
        }
      }
    }
    // Aguarda término dos 20ms precisos
    vTaskDelayUntil(&xLastWakeTime, xFrequency); 
  }
}

// ========= TASK 2: IA E REDE (PRIORIDADE BAIXA) =========
void taskIA_Rede(void *pvParameters) {
  for(;;) {
    int passoMsg;
    // Dispara requisições pendentes da contagem de passos
    while(xQueueReceive(filaPassos, &passoMsg, 0) == pdPASS) {
      sendPost(1);
      Serial.print("Passo Detetado! Total: ");
      Serial.println(totalPassos);
    }

    // Aguarda notificação de que a matriz de 100 amostras encheu
    if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(50)) > 0) {
      float features[12];
      for (int eixo = 0; eixo < 6; eixo++) {
        float soma = 0;
        for (int i = 0; i < TAMANHO_JANELA; i++) {
          soma += buffersIA[bufferProcessamento][i][eixo];
        }
        float media = soma / TAMANHO_JANELA;
        
        float somaVariancia = 0;
        for (int i = 0; i < TAMANHO_JANELA; i++) {
          somaVariancia += pow(buffersIA[bufferProcessamento][i][eixo] - media, 2);
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
  }
}

// ========= TASK 3: INTERFACE OLED (PRIORIDADE MÉDIA) =========
void taskDisplay(void *pvParameters) {
  for(;;) {
    if (xSemaphoreTake(mutexI2C, portMAX_DELAY) == pdTRUE) {
      u8g2.clearBuffer();
      u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
      u8g2.setFont(u8g2_font_spleen12x24_me);
      u8g2.print(totalPassos);
      u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
      u8g2.setFont(u8g2_font_bauhaus2015_tr);
      u8g2.print("Passos");
      u8g2.sendBuffer();
      xSemaphoreGive(mutexI2C);
    }
    vTaskDelay(pdMS_TO_TICKS(INTERVALO_DISPLAY));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);

  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setFont(u8g2_font_spleen12x24_me);

  u8g2.clearBuffer();
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 20);
  u8g2.setFont(u8g2_font_bauhaus2015_tr);
  u8g2.print("Buscando");
  u8g2.setCursor(X_OFFSET + 5, Y_OFFSET + 40);
  u8g2.print("Wifi...");
  u8g2.sendBuffer();

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("Patinhas-Config")) {
    Serial.println("Falha ao conectar. Reiniciando...");
    delay(3000);
    ESP.restart();
  }

  WiFi.setSleep(false);
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

  // Inicialização das estruturas do FreeRTOS
  mutexI2C = xSemaphoreCreateMutex();
  filaPassos = xQueueCreate(10, sizeof(int));

  // Criação das Threads. O terceiro parâmetro é o tamanho da Stack em bytes.
  xTaskCreate(taskSensor, "Sensor", 4096, NULL, 3, NULL);
  xTaskCreate(taskIA_Rede, "IA_Rede", 8192, NULL, 1, &handleTaskIA);
  xTaskCreate(taskDisplay, "Display", 2048, NULL, 2, NULL);
}

void loop() {
  // Deleta a thread principal vazia para liberar memória SRAM
  vTaskDelete(NULL); 
}
