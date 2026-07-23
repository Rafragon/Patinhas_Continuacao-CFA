# Sistema Patinhas-CFA - Documentação geral da Aplicação

Este repositório contém a atualização da infraestrutura de software do projeto Patinhas-CFA, compreendendo o backend em Flask, o frontend em React Native (Expo) e os algoritmos de processamento de sinais e Inteligência Artificial embarcados.

---

## 1. Estrutura Geral do Sistema

A aplicação funciona como um ecossistema distribuído de monitoramento de pets:
1. **Coleira (ESP32-C3 + MPU9250):** Coleta dados inerciais a 50Hz, calcula passos via heurística determinística e classifica comportamento via Random Forest embarcado, enviando os resultados via HTTP POST. O sistema também realiza a leitura analógica do nível de bateria.
2. **Backend (Flask Server):** Escuta as requisições do hardware, persiste as métricas em formato JSON e disponibiliza endpoints de leitura para a interface.
3. **Frontend (React Native / Expo Web):** Consome os dados do backend em tempo real e exibe indicadores gráficos para o usuário diretamente no navegador do computador.

---

## 2. Backend (Servidor Flask)

O servidor local gerencia o fluxo de dados e a persistência do estado do animal.

### Arquivos Principais
* `server.py`: Script principal que inicializa o servidor Flask na porta 8080 e gerencia as rotas.
* `animais.json`: Base de dados não-relacional simplificada para armazenamento permanente

### Endpoints da API

#### Rotas de Escrita (Utilizadas pelo Hardware)
* **`POST /`**: Recebe o sinal de que um passo foi computado pelo ESP32. Verifica a data atual do sistema e atualiza ou cria o registro diário de passos no arquivo `animais.json`.
* **`POST /estado`**: Recebe a classificação comportamental gerada pela Inteligência Artificial do dispositivo. Sobrescreve a chave `estadoAtual` do pet e atualiza o histórico diário com a contagem do estado correspondente.
* **`POST /bateria`**: Recebe o valor percentual da bateria e atualiza a chave respectiva do pet no banco de dados.
* **`POST /ipColeira`**: Recebe e salva o endereço IP do microcontrolador no registro do pet.

#### Rotas de Leitura (Utilizadas pelo Frontend)
* **`GET /passosDia`**: Retorna o histórico consolidado de passos diários do pet buscando o parâmetro de data fornecido.
* **`GET /estado`**: Retorna o estado de atividade mais recente obtido pela IA. Retorna um valor padrão caso o campo não exista.
* **`GET /animalInfo`**: Devolve as informações integrais do arquivo JSON referentes aos pets.
* **`GET /ipColeira`**: Retorna o endereço IP dinâmico atribuído à coleira registrado no servidor.

---

## 3. Frontend (Interface Expo)

O aplicativo foi desenvolvido em React Native utilizando a plataforma Expo, configurado para execução via navegador web e compilação nativa para smartphones Android através do Expo Dev Client.

### Funcionamento e Sincronização
* A interface consome os endpoints do backend local via requisições HTTP assíncronas. A resolução de IP utiliza um hook para conectar-se ao servidor Flask rodando localmente.
* O componente core implementa um mecanismo de pooling com intervalos estritos de 2000 milissegundos (2 segundos).
* A cada ciclo do timer, o frontend busca o estado de atividade atual, o somatório de passos do dia e solicita o nível percentual de carga da bateria do dispositivo de hardware.
* O dashboard em tempo real renderiza o status de ação em destaque e um indicador visual correspondente à porcentagem de bateria.
* A tela de provisionamento IoT exibe instruções textuais para guiar a conexão manual do computador à rede `Patinhas-Config` e realizar o provisionamento através do IP correspondente no navegador.

---

## 4. Inteligência Artificial, Contador de Passos e Bateria

A lógica de processamento foi projetada para execução de baixo custo computacional diretamente na borda (Edge).

### Arquivo Principal
* `patinhas_ia.ino`: programa que roda dentro da Coleira e contém o Contador de Passos e o Classificador de Comportamento.

### 4.1. Algoritmo do Contador de Passos (Heurística)
Diferente do classificador estatístico, a contagem de passos baseia-se em um modelo matemático determinístico executado a cada amostra (50Hz):
1. **Magnitude Vetorial:** Calcula a aceleração linear consolidada nos três eixos espaciais através de raiz quadrada.
2. **Média Móvel:** Aplica um filtro digital de profundidade 3 sobre as leituras para remover ruídos de alta frequência (jitter).
3. **Máquina de Estados de Picos:** Identifica um passo válido quando a aceleração filtrada ultrapassa o limiar superior de 1.2G (pico de impacto) e, sequencialmente, cruza abaixo do limiar inferior de 0.95G (vale de acomodação).
4. **Filtro de Giroscópio:** Para prevenir falsos-positivos provocados por chacoalhar de cabeça ou movimentos isolados, o algoritmo monitora os eixos do giroscópio. Se a taxa de rotação bruta superar 100 graus por segundo, o passo atual é descartado.
5. **Restrição Temporal:** É exigido um tempo mínimo obrigatório de 100 milissegundos entre o registro de passos.
### 4.2. Classificador de Comportamento (Random Forest)
Responsável por mapear se o animal está estacionário ou realizando movimentos cíclicos.
* **Segmentação de Dados (Windowing):** O sensor MPU9250 realiza leituras estruturadas em intervalos regulares de 20ms (frequência de 50Hz). O algoritmo agrupa essas leituras em janelas temporais de 100 amostras consecutivas, representando exatamente 2 segundos de atividade.
* **Extração de Características (Feature Extraction):** Para cada janela de 2 segundos, são calculadas 12 variáveis estatísticas: a Média Aritmética e o Desvio Padrão de cada um dos 6 eixos de dados.
* **Execução Local:** O modelo foi previamente treinado em ambiente Python utilizando o algoritmo `RandomForestClassifier`. A estrutura foi traduzida para C++ permitindo inferência local sem consumo de banda de rede.

### 4.3. Monitoramento de Bateria
* O microcontrolador lê o estado bruto da bateria utilizando sua porta analógica.
* O sinal lido é mapeado para um intervalo de zero a cem para determinar a porcentagem, limitando valores extremos através de condicionais no código.
* A requisição de rede para a atualização da bateria é enviada ao servidor a cada 30 execuções do modelo de Inteligência Artificial.

---

## 5. Como Executar a Aplicação (Via Web)

### Pré-requisitos
* Node.js instalado.
* Python 3.x instalado.
* Arduino ligado.
* Navegador web.

### Passo 1: Inicializar a Coleira (Arduino)
1. Espere o ESP32 se conectar ao Wifi.
2. Se ele não se conectar, acesse o Access Point criado por ele chamado de "Patinhas-Config" e conecte o Wifi desejado através dela no ESP32 (o mesmo Wifi do computador que será utilizado).
3. Após conectar com sucesso à rede local, a placa executa o envio de seu IP local para o servidor Python.

### Passo 2: Inicializar o Backend
1. Acesse a pasta raiz do frontend pelo terminal.
2. Entre numa máquina virtual (venv) dentro da pasta que contém o `server.py` e instale as dependências através do comando `pip install -r requirements_serv.txt`.
3. Inicialize o servidor executando `python server.py`.
4. O servidor começará a escutar requisições HTTP na porta 8080 e broadcast UDP na porta 50000.

### Passo 3: Inicializar o Frontend
1. Acesse a pasta raiz do frontend pelo terminal.
2. Instale as dependências estruturais do projeto executando `npm install`.
3. Inicialize o servidor de desenvolvimento do Expo com `npx expo start`.
4. Pressione a tecla `w` para abrir a aplicação no navegador web do seu computador.

## 6. Como Executar a Aplicação (Via Mobile (Aplicativo React Native / Android))

### Pré-requisitos
* Node.js instalado.
* Python 3.x instalado.
* EAS CLI instalado (`npm install -g eas-cli`) e conta autenticada (`eas login`).
* Arduino (ESP32) ligado.
* Smartphone Android.

### Passo 1: Inicializar a Coleira (Arduino)
1. Espere o ESP32 se conectar ao Wi-Fi.
2. Se ele não se conectar, acesse o Access Point criado por ele chamado de "Patinhas-Config" e conecte o Wi-Fi desejado através dela no ESP32 (o mesmo Wi-Fi do computador que será utilizado).
3. Após conectar com sucesso à rede local, a placa executa o envio de seu IP local para o servidor Python.

### Passo 2: Inicializar o Backend e Configurar Rede
1. Acesse a pasta do backend pelo terminal.
2. Ative uma máquina virtual (venv) e instale as dependências através do comando `pip install -r requirements_serv.txt`.
3. Inicialize o servidor executando `python server.py`.
4. Anote o IP local exibido no terminal e certifique-se de liberar as portas 8080 e 8081 no Firewall do Windows.

### Passo 3: Compilar e Instalar o Aplicativo Nativo
1. Acesse a pasta raiz do frontend pelo terminal.
2. Instale as dependências estruturais executando `npm install`.
3. Gere o development build com o comando `eas build --profile development --platform android`.
4. Faça o download do arquivo `.apk` gerado pelo link no terminal e instale-o no smartphone.

### Passo 4: Inicializar o Frontend e Conectar
1. Abra o arquivo `Aplicacao/frontend/patinhas-components/useServerIP.tsx` e atualize o endereço IP com o valor anotado no Passo 2.
2. Inicie o servidor do Expo executando `npx expo start --dev-client --tunnel`.
3. Abra o aplicativo instalado no smartphone, selecione "Scan QR Code" e escaneie o código do terminal para carregar a interface.