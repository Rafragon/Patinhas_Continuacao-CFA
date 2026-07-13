# Detalhamento do Firmware ESP32-C3

Este diretório contém o código-fonte em C++ para o microcontrolador ESP32-C3. O firmware é responsável pela aquisição de dados do sensor inercial, processamento de sinais para contagem de passos, inferência de estado comportamental via Inteligência Artificial na borda (Edge AI) e comunicação de rede.

---

## 1. Arquitetura de Hardware

O sistema utiliza o barramento I2C para comunicação com os periféricos, multiplexando os dados através de endereços físicos:
*   **Microcontrolador:** ESP32-C3.
*   **Sensor Inercial:** MPU9250 (Endereço I2C `0x68`).
*   **Display:** OLED SSD1306 128x64 (Pinos: `SDA = 5`, `SCL = 6`).

---

## 2. Detalhamento das Interfaces de Comunicação e Rede

O código do ESP32 gerencia três camadas distintas de conectividade. Abaixo está o detalhamento técnico de cada protocolo e suas respectivas implementações.

### 2.1. Camada de Enlace e Configuração (Wi-Fi)
O gerenciamento da rede sem fio é feito pela biblioteca `WiFiManager` e instruções nativas da biblioteca `WiFi.h`.
*   **Ponto de Acesso (AP) e Portal Cativo:** A instrução `wifiManager.autoConnect("Patinhas-Config")` tenta autenticar o ESP32 no roteador utilizando as últimas credenciais salvas na memória não volátil (NVS). Se houver falha, a placa interrompe o fluxo normal e cria sua própria rede Wi-Fi (Access Point) chamada "Patinhas-Config". Ao conectar nessa rede pelo celular, um portal cativo intercepta o tráfego e abre uma página web para a inserção da senha do roteador local.
*   **Prevenção de Latência (Sleep Mode):** Logo após a conexão, o comando `WiFi.setSleep(false)` é executado. O microcontrolador possui mecanismos padrão de economia de energia que desligam a antena do rádio intermitentemente. Como a aplicação exige estabilidade para autodescoberta e envios frequentes, desativar o modo de hibernação previne latência (ping alto) e desconexões.

### 2.2. Protocolo de Autodescoberta (UDP Broadcast)
O sistema utiliza o protocolo UDP (User Datagram Protocol) de forma orientada a broadcast para localizar o servidor Python sem exigir um endereço de IP fixo pré-compilado.
*   **Natureza Connectionless:** O UDP permite o envio imediato de datagramas para a rede sem a necessidade de estabelecer uma conexão prévia (handshake).
*   **Mecanismo de Broadcast:** A comunicação é instanciada na porta `50000`. O pacote de solicitação é disparado para o endereço de IP universal de broadcast da sub-rede local (`255.255.255.255`). O roteador replica a string de texto `DISCOVER_SERVER` para todos os dispositivos conectados à mesma rede.
*   **Filtro e Tratamento de Resposta (Parse):** O ESP32 monitora a porta UDP. Ao identificar um pacote de entrada, a função `udp.parsePacket()` captura seu tamanho. O código avalia os primeiros 10 caracteres da carga útil com a instrução `strncmp(buf, "SERVER_IP:", 10)`. Se a assinatura for validada, o endereço de IP do servidor é extraído do restante do texto e alocado dinamicamente na variável de destino.

### 2.3. Transmissão de Dados da Aplicação (HTTP/TCP)
A comunicação oficial entre o hardware e o backend é realizada através de requisições HTTP POST convencionais. O HTTP opera sobre o protocolo TCP, o que garante confiabilidade e retransmissão de dados perdidos.
*   **Gestão de Sessões Estrita:** Em ambas as funções de envio (`sendPost` e `sendEstadoPost`), um novo objeto `HTTPClient` é instanciado, os dados são transmitidos, e a conexão é finalizada imediatamente via `http.end()`. Essa arquitetura impede o acúmulo de sockets abertos na memória do ESP32 e saturação (hang) no servidor Flask.
*   **Roteamento Direcionado (Endpoints):**
    *   **Contagem de Passos:** A função `sendPost(int mensagem)` envia incrementos inteiros formatados sob o cabeçalho `text/plain` diretamente para a rota raiz (`/`) apontando para a porta `8080` do IP descoberto.
    *   **Inferência (Machine Learning):** A função `sendEstadoPost(String estado)` transmite o resultado nominal do processamento (ex: "andando") para um endpoint secundário nomeado `/estado`. A transmissão da string classificada pelo Random Forest ocorre em intervalos derivados da janela matemática de 100 amostras.

---

## 3. Aquisição de Dados e Processamento de Sinais

O loop principal opera sob uma restrição temporal estrita para garantir a eficácia do modelo de Machine Learning.

### 3.1. Controle de Amostragem (50Hz)
A leitura dos dados não utiliza bloqueios de delay. O fluxo é condicionado pela função `millis()`. A condição `if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS)` garante que o sensor seja lido exatamente a cada 20 milissegundos, resultando em uma taxa de amostragem cravada em 50Hz.

### 3.2. Contagem de Passos (Heurística)
O acelerômetro MPU9250 é lido nos três eixos (X, Y e Z) com escala de $\pm2G$. O giroscópio opera na escala de $\pm250$ graus por segundo.

1.  **Cálculo de Magnitude:** Os vetores individuais são combinados para encontrar a magnitude vetorial absoluta:
    $$A = \sqrt{rawAccX^2 + rawAccY^2 + rawAccZ^2}$$
2.  **Filtro:** Um filtro de média móvel com janela de 3 amostras é aplicado ao valor $A$ para suavizar ruídos mecânicos de alta frequência.
3.  **Máquina de Estados:** Um passo é validado se a magnitude filtrada ultrapassar o limiar de impacto ($1.2G$) e, subsequentemente, cair abaixo do limiar de repouso ($0.95G$).
4.  **Bloqueio por Rotação:** Se a magnitude do giroscópio exceder o limiar de $100.0$ graus por segundo, o incremento de passos entra em um período de espera (cooldown) de 500 milissegundos para evitar a contabilização de movimentos anômalos, como o chacoalhar do animal. Cada passo validado dispara uma requisição HTTP POST para o servidor.

---

## 4. Inteligência Artificial na Borda (Edge AI) e Administração de Memória

O microcontrolador roda um modelo Random Forest convertido para C++ (`ModeloPatinhas.h`).

### 4.1. Administração de Memória e Estrutura Circular
O fluxo de preenchimento dos dados do sensor foi projetado para evitar problemas de alocação de memória no hardware:
*   **Prevenção de Vazamento (Memory Leaks):** A matriz bidimensional (`float leiturasIA[TAMANHO_JANELA][6]`) e o vetor estatístico (`float features[12]`) que alimentam o classificador são criados com alocação estática. O código recicla essas mesmas posições físicas de memória na SRAM. Evitar métodos de alocação dinâmica (como `malloc` ou vetores mutáveis) previne o problema crônico de fragmentação de memória (Heap Fragmentation), assegurando que o microcontrolador possa rodar semanas em loop contínuo sem travamentos.
*   **Extração de Características:** Os dados brutos são empilhados na matriz `leiturasIA` até atingir o limite de 100 amostras (2 segundos).
*   Ao preencher a janela, a função `processarIA()` utiliza os ciclos da CPU para calcular de forma isolada 12 features estáticas: a média aritmética ($\mu$) e o desvio padrão ($\sigma$) para cada um dos 6 eixos inerciais coletados.
*   O resultado das matrizes flutuantes é processado através da função do modelo em árvore `modelo.predictLabel(features)`, e em seguida, o índice `indiceIA` é resetado nativamente, reiniciando o empilhamento das variáveis em cima dos últimos valores contidos nas matrizes, sem requerer ações de formatação ou sobreposição no espaço interno.# Firmware ESP32-C3 - Projeto Patinhas

Este diretório contém o código-fonte em C++ para o microcontrolador ESP32-C3. O firmware é responsável pela aquisição de dados do sensor inercial, processamento de sinais para contagem de passos, inferência de estado comportamental via Inteligência Artificial na borda (Edge AI) e comunicação de rede.

---

## 1. Arquitetura de Hardware

O sistema utiliza o barramento I2C para comunicação com os periféricos, multiplexando os dados através de endereços físicos:
*   **Microcontrolador:** ESP32-C3.
*   **Sensor Inercial:** MPU9250 (Endereço I2C `0x68`).
*   **Display:** OLED SSD1306 128x64 (Pinos: `SDA = 5`, `SCL = 6`).

---

## 2. Detalhamento das Interfaces de Comunicação e Rede

O código do ESP32 gerencia três camadas distintas de conectividade. Abaixo está o detalhamento técnico de cada protocolo e suas respectivas implementações.

### 2.1. Camada de Enlace e Configuração (Wi-Fi)
O gerenciamento da rede sem fio é feito pela biblioteca `WiFiManager` e instruções nativas da biblioteca `WiFi.h`.
*   **Ponto de Acesso (AP) e Portal Cativo:** A instrução `wifiManager.autoConnect("Patinhas-Config")` tenta autenticar o ESP32 no roteador utilizando as últimas credenciais salvas na memória não volátil (NVS). Se houver falha, a placa interrompe o fluxo normal e cria sua própria rede Wi-Fi (Access Point) chamada "Patinhas-Config". Ao conectar nessa rede pelo celular, um portal cativo intercepta o tráfego e abre uma página web para a inserção da senha do roteador local.
*   **Prevenção de Latência (Sleep Mode):** Logo após a conexão, o comando `WiFi.setSleep(false)` é executado. O microcontrolador possui mecanismos padrão de economia de energia que desligam a antena do rádio intermitentemente. Como a aplicação exige estabilidade para autodescoberta e envios frequentes, desativar o modo de hibernação previne latência (ping alto) e desconexões.

### 2.2. Protocolo de Autodescoberta (UDP Broadcast)
O sistema utiliza o protocolo UDP (User Datagram Protocol) de forma orientada a broadcast para localizar o servidor Python sem exigir um endereço de IP fixo pré-compilado.
*   **Natureza Connectionless:** O UDP permite o envio imediato de datagramas para a rede sem a necessidade de estabelecer uma conexão prévia (handshake).
*   **Mecanismo de Broadcast:** A comunicação é instanciada na porta `50000`. O pacote de solicitação é disparado para o endereço de IP universal de broadcast da sub-rede local (`255.255.255.255`). O roteador replica a string de texto `DISCOVER_SERVER` para todos os dispositivos conectados à mesma rede.
*   **Filtro e Tratamento de Resposta (Parse):** O ESP32 monitora a porta UDP. Ao identificar um pacote de entrada, a função `udp.parsePacket()` captura seu tamanho. O código avalia os primeiros 10 caracteres da carga útil com a instrução `strncmp(buf, "SERVER_IP:", 10)`. Se a assinatura for validada, o endereço de IP do servidor é extraído do restante do texto e alocado dinamicamente na variável de destino.

### 2.3. Transmissão de Dados da Aplicação (HTTP/TCP)
A comunicação oficial entre o hardware e o backend é realizada através de requisições HTTP POST convencionais. O HTTP opera sobre o protocolo TCP, o que garante confiabilidade e retransmissão de dados perdidos.
*   **Gestão de Sessões Estrita:** Em ambas as funções de envio (`sendPost` e `sendEstadoPost`), um novo objeto `HTTPClient` é instanciado, os dados são transmitidos, e a conexão é finalizada imediatamente via `http.end()`. Essa arquitetura impede o acúmulo de sockets abertos na memória do ESP32 e saturação (hang) no servidor Flask.
*   **Roteamento Direcionado (Endpoints):**
    *   **Contagem de Passos:** A função `sendPost(int mensagem)` envia incrementos inteiros formatados sob o cabeçalho `text/plain` diretamente para a rota raiz (`/`) apontando para a porta `8080` do IP descoberto.
    *   **Inferência (Machine Learning):** A função `sendEstadoPost(String estado)` transmite o resultado nominal do processamento (ex: "andando") para um endpoint secundário nomeado `/estado`. A transmissão da string classificada pelo Random Forest ocorre em intervalos derivados da janela matemática de 100 amostras.

---

## 3. Aquisição de Dados e Processamento de Sinais

O loop principal opera sob uma restrição temporal estrita para garantir a eficácia do modelo de Machine Learning.

### 3.1. Controle de Amostragem (50Hz)
A leitura dos dados não utiliza bloqueios de delay. O fluxo é condicionado pela função `millis()`. A condição `if (currentTime - lastSampleTime >= SAMPLE_INTERVAL_MS)` garante que o sensor seja lido exatamente a cada 20 milissegundos, resultando em uma taxa de amostragem cravada em 50Hz.

### 3.2. Contagem de Passos (Heurística)
O acelerômetro MPU9250 é lido nos três eixos (X, Y e Z) com escala de $\pm2G$. O giroscópio opera na escala de $\pm250$ graus por segundo.

1.  **Cálculo de Magnitude:** Os vetores individuais são combinados para encontrar a magnitude vetorial absoluta:
    $$A = \sqrt{rawAccX^2 + rawAccY^2 + rawAccZ^2}$$
2.  **Filtro:** Um filtro de média móvel com janela de 3 amostras é aplicado ao valor $A$ para suavizar ruídos mecânicos de alta frequência.
3.  **Máquina de Estados:** Um passo é validado se a magnitude filtrada ultrapassar o limiar de impacto ($1.2G$) e, subsequentemente, cair abaixo do limiar de repouso ($0.95G$).
4.  **Bloqueio por Rotação:** Se a magnitude do giroscópio exceder o limiar de $100.0$ graus por segundo, o incremento de passos entra em um período de espera (cooldown) de 500 milissegundos para evitar a contabilização de movimentos anômalos, como o chacoalhar do animal. Cada passo validado dispara uma requisição HTTP POST para o servidor.

---

## 4. Inteligência Artificial na Borda (Edge AI) e Administração de Memória

O microcontrolador roda um modelo Random Forest convertido para C++ (`ModeloPatinhas.h`).

### 4.1. Administração de Memória e Estrutura Circular
O fluxo de preenchimento dos dados do sensor foi projetado para evitar problemas de alocação de memória no hardware:
*   **Prevenção de Vazamento (Memory Leaks):** A matriz bidimensional (`float leiturasIA[TAMANHO_JANELA][6]`) e o vetor estatístico (`float features[12]`) que alimentam o classificador são criados com alocação estática. O código recicla essas mesmas posições físicas de memória na SRAM. Evitar métodos de alocação dinâmica (como `malloc` ou vetores mutáveis) previne o problema crônico de fragmentação de memória (Heap Fragmentation), assegurando que o microcontrolador possa rodar semanas em loop contínuo sem travamentos.
*   **Extração de Características:** Os dados brutos são empilhados na matriz `leiturasIA` até atingir o limite de 100 amostras (2 segundos).
*   Ao preencher a janela, a função `processarIA()` utiliza os ciclos da CPU para calcular de forma isolada 12 features estáticas: a média aritmética ($\mu$) e o desvio padrão ($\sigma$) para cada um dos 6 eixos inerciais coletados.
*   O resultado das matrizes flutuantes é processado através da função do modelo em árvore `modelo.predictLabel(features)`, e em seguida, o índice `indiceIA` é resetado nativamente, reiniciando o empilhamento das variáveis em cima dos últimos valores contidos nas matrizes, sem requerer ações de formatação ou sobreposição no espaço interno.
