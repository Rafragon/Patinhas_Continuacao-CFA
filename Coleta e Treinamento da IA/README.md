# Sistema Patinhas-CFA - Coleta e Treinamento da Inteligência Artificial

Este diretório contém os scripts necessários para capturar dados inerciais do ESP32-C3 e treinar o modelo de Machine Learning (Random Forest) embarcado na coleira inteligente. O fluxo de trabalho inclui coleta, segmentação em janelas temporais de 2 segundos, treinamento e conversão para código C++ estático.

---

## 1. Dependências do Ambiente

Para replicar a infraestrutura, instale os pacotes e bibliotecas listados abaixo.

### 1.1. Dependências de Firmware (Arduino IDE)
As seguintes bibliotecas devem ser instaladas na IDE para compilar o código do ESP32-C3:
*   `Wire`: Nativa para comunicação I2C.
*   `WiFi` e `WiFiManager`: Para gerenciamento de conexões sem fio e criação do portal cativo.
*   `MPU9250`: Para leitura do acelerômetro e giroscópio.
*   `U8g2`: Para interface visual no display OLED SSD1306.

### 1.2. Dependências Python
Instale os pacotes no ambiente virtual via terminal:
```bash
pip install -r requirements_tr.txt
```
*A biblioteca `pyserial` gerencia a coleta via cabo, enquanto o restante processa os dados e converte o modelo. O script de rede utiliza a biblioteca nativa `socket`.*

---

## 2. Estrutura dos Arquivos

*   `coleta_serial.py`: Captura dados via porta serial (cabo USB).
*   `coleta_wifi.py`: Captura dados via rede Wi-Fi local através de protocolo TCP.
*   `treinamento_ia.py`: Lê os arquivos CSV, extrai características, treina o classificador e exporta a matriz C++.
*   `coleta_serial.ino`: Firmware para amostragem e envio a 50Hz via USB.
*   `coleta_wifi.ino`: Firmware para amostragem a 50Hz via Wi-Fi, gerando a rede `Patinhas-Coleta`.
*   `inferencia_local.ino`: Firmware para testar o modelo treinado executando a predição na própria placa.

---

## 3. Coleta de Dados Brutos

O sistema mapeia o comportamento do animal utilizando o cabeçalho estrito `accX,accY,accZ,gyroX,gyroY,gyroZ`.

### 3.1. Calibração Inicial do Hardware
Ambos os firmwares de coleta (`coleta_serial.ino` e `coleta_wifi.ino`) executam a função `mpu.calibrateAccelGyro()` no bloco `setup()`. O dispositivo deve ser mantido completamente imóvel durante a inicialização até que o display ou o terminal confirmem o fim da calibração.

### 3.2. Gravação via Wi-Fi (Recomendado)
1. Faça o upload do arquivo `coleta_wifi.ino`.
2. O ESP32 criará o Access Point `Patinhas-Coleta` se não localizar uma rede salva. Conecte-se e insira as credenciais do Wi-Fi local.
3. O display OLED dividirá o IP obtido em duas linhas para facilitar a leitura.
4. No arquivo `coleta_wifi.py`, configure as variáveis `IP_ESP32` com o IP exibido na tela, defina o `NOME_DO_ARQUIVO` (ex: `dados_correndo.csv`) e mantenha a porta TCP `80`.
5. Execute o script com o comando `python coleta_wifi.py`. O socket abrirá a conexão e gravará os dados continuamente até o limite da variável `TEMPO_DE_COLETA` (padrão: 30 segundos).

### 3.3. Gravação via Cabo Serial
1. Faça o upload do arquivo `coleta_serial.ino` e feche o Monitor Serial da IDE.
2. No script `coleta_serial.py`, defina a variável `PORTA_SERIAL` com a porta correspondente do sistema operacional e `BAUD_RATE` em `115200`.
3. Execute `python coleta_serial.py`. O script descartará mensagens iniciais até reconhecer a string de cabeçalho `accX`, marcando o tempo zero para iniciar a escrita.

---

## 4. Processamento e Treinamento do Modelo

1. Crie um diretório chamado `dados` na mesma pasta do script de treinamento.
2. Mova os arquivos CSV gravados para este diretório. O arquivo `treinamento_ia.py` busca explicitamente chaves correspondentes a `dados_parado.csv`, `dados_andando.csv`, `dados_correndo.csv` e `dados_se_cocando.csv`.
3. Execute `python treinamento_ia.py`.
4. **Extração de Características:** O código segmenta as linhas do CSV em blocos de 100 amostras (janelas exatas de 2 segundos em 50Hz). Para cada bloco, são extraídas 12 variáveis independentes: a média e o desvio padrão geométrico de cada um dos 6 eixos.
5. O classificador `RandomForestClassifier` é treinado com 30 estimadores (`n_estimators=30`) e profundidade máxima de 10 (`max_depth=10`).
6. A biblioteca `micromlgen` converte as matrizes de decisão resultantes e gera o arquivo `ModeloPatinhas.h` no diretório raiz.

---

## 5. Validação Edge AI (Microcontrolador)

1. Aloque o arquivo recém-gerado `ModeloPatinhas.h` na pasta do firmware `inferencia_local.ino`.
2. Compile e transfira o código para a placa.
3. O firmware criará uma matriz `leituras[100][6]` em memória. O acelerômetro gravará amostras a cada 20 milissegundos.
4. Quando o índice atingir 100, a função `classificarMovimento()` processará as mesmas 12 características estatísticas locais (6 médias e 6 desvios).
5. A função `modelo.predictLabel(features)` executará a árvore de decisão embarcada e imprimirá a string correspondente no Monitor Serial.