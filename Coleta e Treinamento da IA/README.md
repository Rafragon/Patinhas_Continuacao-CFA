# Sistema Patinhas-CFA - Coleta e Treinamento da Inteligência Artificial

Este diretório contém os scripts necessários para capturar dados inerciais do ESP32-C3 e treinar o modelo de Machine Learning (Random Forest) que será embarcado na coleira inteligente.

---

## 1. Visão Geral do Fluxo de Trabalho

Para que a coleira consiga identificar autonomamente o que o animal está fazendo, o sistema precisa passar por um ciclo de aprendizado supervisionado:
1. **Coleta de Dados:** O ESP32 envia dados brutos dos sensores de movimento para o computador, enquanto o animal executa uma ação específica.
2. **Processamento:** O script em Python segmenta esses dados em janelas temporais de 2 segundos.
3. **Treinamento:** O algoritmo `RandomForestClassifier` aprende a diferenciar os padrões de cada movimento.
4. **Conversão (Edge AI):** O modelo validado é traduzido para código C++ (`ModeloPatinhas.h`) e incluído no firmware final do dispositivo.

---

## 2. Estrutura dos Arquivos

* `coleta_serial.py`: Script para coletar dados do ESP32 através de conexão via cabo USB.
* `coleta_wifi.py`: Script para coletar dados do ESP32 através de rede Wi-Fi local via protocolo TCP (Recomendado para o pet se mover livremente).
* `treinamento_ia.py`: Script responsável por ler os arquivos CSV coletados, treinar a floresta aleatória e exportar o modelo para a linguagem C++.
* `coleta_serial.ino`: Firmware do ESP32 para enviar dados em alta frequência (50Hz) pela porta serial (cabo USB).
* `coleta_wifi.ino`: Firmware do ESP32 para transmitir os dados inerciais via Wi-Fi a 50Hz, incluindo integração com o display OLED e WiFiManager.
* `inferencia_local.ino`: Firmware de teste para validar o funcionamento do modelo de IA embarcado no microcontrolador, imprimindo os resultados no Monitor Serial.

---

## 3. Passo a Passo da Coleta de Dados

Para cada comportamento que você deseja ensinar (ex: *parado*, *andando*, *correndo*, *se_cocando*), siga estas etapas:

### 3.1. Preparando o Hardware
1. Abra o arquivo `coleta_wifi.ino` na Arduino IDE.
2. Certifique-se de que a opção "USB CDC On Boot" está como "Enabled" (se utilizar cabo) e faça o upload para o ESP32-C3.
3. Ligue a coleira. Caso ela não encontre uma rede, criará um Access Point chamado `Patinhas-Coleta`. Conecte-se a ele pelo celular e insira a senha do seu Wi-Fi.
4. O display OLED irá exibir o IP que a coleira obteve na rede local. Anote este IP.

### 3.2. Gravando os Dados
1. No seu computador, abra o arquivo `coleta_wifi.py` e altere a variável `IP_ESP32` para o IP mostrado no display da coleira.
2. Altere a variável `NOME_DO_ARQUIVO` para refletir a ação que será gravada (ex: `dados_correndo.csv`). O tempo padrão de coleta é de 30 segundos.
3. Execute o script no terminal: `python coleta_wifi.py`
4. Estimule o animal a executar a ação desejada de forma contínua enquanto o script grava os dados brutos. O arquivo CSV será salvo na mesma pasta do script.

*(Nota: Caso prefira utilizar cabo, faça o upload de `coleta_serial.ino`, feche o monitor serial da IDE, configure a variável `PORTA_SERIAL` em `coleta_serial.py` e execute o script python.)*

---

## 4. Treinamento e Conversão do Modelo

Após coletar os arquivos CSV para todos os comportamentos desejados, é hora de treinar a Inteligência Artificial.

1. **Organização:** Crie uma pasta chamada `dados` dentro deste diretório e mova todos os arquivos CSV gerados para lá.
2. **Dependências:** Certifique-se de instalar as bibliotecas necessárias:
```bash
pip install -r requirements.txt
```
3. **Mapeamento:** Abra o arquivo `treinamento_ia.py` e verifique o dicionário `ARQUIVOS` para garantir que as chaves correspondam aos nomes dos seus arquivos na pasta `dados`.
   ```python
   ARQUIVOS = {
       'parado': os.path.join(PASTA_DADOS, 'dados_parado.csv'),
       'andando': os.path.join(PASTA_DADOS, 'dados_andando.csv'),
       'correndo': os.path.join(PASTA_DADOS, 'dados_correndo.csv'),
       'se_cocando': os.path.join(PASTA_DADOS, 'dados_se_cocando.csv')
   }
   ```
4. **Execução:** Rode o script: `python treinamento_ia.py`
5. **Resultado:** O console imprimirá a acurácia do modelo treinado. Um novo arquivo chamado `ModeloPatinhas.h` será gerado. Este arquivo contém a estrutura C++ estática da floresta aleatória.

---

## 5. Validação no Microcontrolador

Para garantir que o treinamento foi bem-sucedido e está funcionando no hardware:

1. Mova o arquivo gerado `ModeloPatinhas.h` para a mesma pasta onde está o firmware `inferencia_local.ino`.
2. Faça o upload do arquivo `inferencia_local.ino` para o ESP32-C3.
3. Abra o Monitor Serial da Arduino IDE.
4. Movimente o dispositivo enquanto ele está conectado no cabo USB; o microcontrolador irá agrupar as leituras e extrair a média e o desvio padrão a cada 2 segundos, inserindo-as no classificador e imprimindo o estado comportamental detetado diretamente na tela.
