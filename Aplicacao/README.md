# Sistema Patinhas-CFA - Documentação da Aplicação

Este repositório contém a atualização da infraestrutura de software do projeto Patinhas-CFA, compreendendo o backend em Flask, o frontend em React Native (Expo) e os algoritmos de processamento de sinais e Inteligência Artificial embarcados.

---

## 1. Estrutura Geral do Sistema

A aplicação funciona como um ecossistema distribuído de monitoramento de pets:
1. **Coleira (ESP32-C3 + MPU9250):** Coleta dados inerciais a 50Hz, calcula passos via heurística determinística e classifica comportamento via Random Forest embarcado, enviando os resultados via HTTP POST.
2. **Backend (Flask Server):** Escuta as requisições do hardware, persiste as métricas em formato JSON e disponibiliza endpoints de leitura para a interface.
3. **Frontend (React Native / Expo Web):** Consome os dados do backend em tempo real e exibe indicadores gráficos para o usuário diretamente no navegador do computador.

---

## 2. Backend (Servidor Flask)

O servidor local gerencia o fluxo de dados e a persistência do estado do animal.

### Arquivos Principais
* `server.py`: Script principal que inicializa o servidor Flask na porta 8080 e gerencia as rotas.
* `animais.json`: Base de dados não-relacional simplificada para armazenamento permanente.

### Endpoints da API

#### Rotas de Escrita (Utilizadas pelo Hardware)
* `POST /`
  * **Função:** Recebe o sinal de que um passo foi computado pelo ESP32.
  * **Comportamento:** Verifica a data atual do sistema (AAAA-MM-DD). Se houver um registro para o dia atual no histórico do arquivo `animais.json`, incrementa o valor de `num_passos`. Caso contrário, cria um novo bloco diário.
* `POST /estado`
  * **Função:** Recebe a classificação comportamental gerada pela Inteligência Artificial do dispositivo.
  * **Payload esperado:** String pura identificando o estado (ex: `parado`, `andando`).
  * **Comportamento:** Sobrescreve a chave `estadoAtual` do pet no arquivo JSON.

#### Rotas de Leitura (Utilizadas pelo Frontend)
* `GET /passosDia`
  * **Função:** Retorna o histórico consolidado de passos diários do pet.
* `GET /estado`
  * **Função:** Retorna o estado de atividade mais recente obtido pela IA.

---

## 3. Frontend (Interface Expo)

O aplicativo foi desenvolvido em React Native utilizando a plataforma Expo, configurado para execução nativa no navegador web do computador.

### Funcionamento e Sincronização
* A interface consome os endpoints do backend local via requisições HTTP assíncronas.
* O componente core `PetStatus.tsx` implementa um mecanismo de pooling com intervalos estritos de 2000 milissegundos (2 segundos).
* A cada ciclo do timer, o frontend busca o estado de atividade atual e o somatório de passos do dia, atualizando o progresso gráfico na tela de forma dinâmica.

---

## 4. Inteligência Artificial e Contador de Passos 

A lógica de processamento foi projetada para execução de baixo custo computacional diretamente na borda (Edge).

### Arquivo Principal
* `patinhas_ia.ino`: programa que roda dentro da Coleira e contém o Contador de Passos e o Classificador de Comportamento.

### 4.1. Algoritmo do Contador de Passos (Heurística)
Diferente do classificador estatístico, a contagem de passos baseia-se em um modelo matemático determinístico executado a cada amostra (50Hz):
1. **Magnitude Vetorial:** Calcula a aceleração linear consolidada nos três eixos espaciais:
   A = sqrt(accX^2 + accY^2 + accZ^2)
2. **Média Móvel:** Aplica um filtro digital de profundidade 3 sobre as leituras para remover ruídos de alta frequência (jitter).
3. **Máquina de Estados de Picos:** Identifica um passo válido quando a aceleração filtrada ultrapassa o limiar superior de 1.2G (pico de impacto) e, sequencialmente, cruza abaixo do limiar inferior de 0.95G (vale de acomodação).
4. **Filtro de Giroscópio:** Para prevenir falsos-positivos provocados por chacoalhar de cabeça ou movimentos isolados, o algoritmo monitora os eixos do giroscópio. Se a taxa de rotação bruta superar 100 graus por segundo, o passo atual é descartado.

### 4.2. Classificador de Comportamento (Random Forest)
Responsável por mapear se o animal está estacionário ou realizando movimentos cíclicos.
* **Segmentação de Dados (Windowing):** O sensor MPU9250 realiza leituras estruturadas em intervalos regulares de 20ms (frequência de 50Hz). O algoritmo agrupa essas leituras em janelas temporais de 100 amostras consecutivas, representando exatamente 2 segundos de atividade.
* **Extração de Características (Feature Extraction):** Para cada janela de 2 segundos, são calculadas 12 variáveis estatísticas: a Média Aritmética e o Desvio Padrão de cada um dos 6 eixos de dados (accX, accY, accZ, gyroX, gyroY, gyroZ).
* **Execução Local:** O modelo foi previamente treinado em ambiente Python através do pacote `scikit-learn` utilizando o algoritmo `RandomForestClassifier` (configurado com 30 estimadores e profundidade limite de 10 níveis). Utilizando a biblioteca `micromlgen`, a estrutura da floresta aleatória foi traduzida para lógica C++ pura dentro do arquivo cabeçalho `ModeloPatinhas.h`, permitindo inferência local ultrarrápida sem consumo de banda de rede.

---

## 5. Como Executar a Aplicação

### Pré-requisitos
* Node.js instalado.
* Python 3.x instalado.
* Arduino ligado.
* Navegador web.

### Passo 1: Inicializar a Coleira (Arduino)
Espere o ESP32 se conectar ao Wifi. Se ele não se conectar, acesse o Access Point criado por ele chamado de "Patinhas-Config" e conecte o Wifi desejado atráves dela no ESP32 (o mesmo Wifi do computador que será utilizado).

### Passo 2: Inicializar o Backend
1. Acesse a pasta raiz do frontend pelo terminal.
2. Entre numa máquina virtual (venv) dentro da pasta que contém o "server.py":
```bash
# Para Windows
.\venv\Scripts\Activate
# ou
# Para Linux
source venv/bin/activate
```
Caso não tenha uma "venv", crie uma:
```bash
python -m venv venv
```
e depois repita a primeira parte.
Para fechar depois utilize:
```bash
deactive
```
3. Instale as dependências:
```bash
pip install -r requirements_serv.txt
```
4. Inicialize o servidor backend:
```bash
python server.py
```
O servidor começará a escutar requisições HTTP na porta 8080 e broadcast UDP na porta 50000.

### Passo 3: Inicializar o Frontend
1. Acesse a pasta raiz do frontend pelo terminal.
2. Instale as dependências estruturais do projeto:
```bash
npm install
```
3. Inicialize o servidor de desenvolvimento do Expo:
```bash
npx expo start
```
4. No terminal, pressione a tecla `w` para abrir a aplicação diretamente no navegador web do seu computador.
