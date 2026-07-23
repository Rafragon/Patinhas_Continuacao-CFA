# Detalhamento do Firmware ESP32-C3

Este diretório contém o código-fonte em C++ para o microcontrolador ESP32-C3. O firmware é responsável pela aquisição de dados do sensor inercial, processamento de sinais para contagem de passos, inferência de estado comportamental via Inteligência Artificial na borda (Edge AI) e comunicação de rede, operando de forma multitarefa sobre o FreeRTOS.

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
*   **Gestão de Sessões Estrita:** Em ambas as funções de envio, um novo objeto `HTTPClient` é instanciado, os dados são transmitidos, e a conexão é finalizada imediatamente via `http.end()`. Essa arquitetura impede o acúmulo de sockets abertos na memória do ESP32 e saturação (hang) no servidor Flask.
*   **Roteamento Direcionado (Endpoints):**
    *   **Contagem de Passos:** A função `sendPost(int mensagem)` envia incrementos inteiros formatados sob o cabeçalho `text/plain` diretamente para a rota raiz (`/`) apontando para a porta `8080` do IP descoberto.
    *   **Inferência (Machine Learning):** A função `sendEstadoPost(String estado)` transmite o resultado nominal do processamento (ex: "andando") para um endpoint secundário nomeado `/estado`. A transmissão da string classificada pelo Random Forest ocorre sob demanda processada.
    *   **Identificação e Bateria:** Rotas auxiliares enviam o IP dinâmico da placa no momento da inicialização (`/ipColeira`) e as atualizações processadas do status de energia (`/bateria`) de forma intervalada.
      
---

## 3. Sistema Operacional de Tempo Real (FreeRTOS)

Para isolar o hardware de instabilidades de rede e manter a amostragem inercial exata, o fluxo contínuo foi estruturado em concorrência baseada em threads (Tasks):

*   **Task 1: Sensor (Prioridade Alta):** Aloca 4096 bytes de Stack. Executa a leitura do acelerômetro e do giroscópio exatamente a cada 20ms utilizando `vTaskDelayUntil`. Contém a heurística de filtro de média móvel para detecção e contagem de passos.
*   **Task 2: IA e Rede (Prioridade Baixa):** Aloca 8192 bytes de Stack. Calcula as características estatísticas, submete os dados ao modelo Random Forest, avalia as métricas da bateria e executa as transmissões HTTP. Roda apenas utilizando o tempo ocioso do processador.
*   **Task 3: Display (Prioridade Média):** Aloca 2048 bytes de Stack. Limpa o buffer e atualiza o display OLED a cada 100ms.

---

## 4. Aquisição de Dados e Processamento de Sinais

A amostragem opera sob uma restrição temporal estrita dentro da Task de prioridade alta para garantir a eficácia do modelo de Machine Learning.

### 4.1. Controle de Amostragem (50Hz)
A leitura dos dados não utiliza bloqueios comuns de delay. O fluxo é condicionado pela instrução `vTaskDelayUntil`, que garante que o sensor seja lido exatamente a cada 20 milissegundos, resultando em uma taxa de amostragem cravada em 50Hz e protegida contra latências externas.

### 4.2. Contagem de Passos (Heurística)
O acelerômetro MPU9250 é lido nos três eixos (X, Y e Z) com escala de ±2G. O giroscópio opera na escala de ±250 graus por segundo.

1.  **Cálculo de Magnitude:** Os vetores individuais são combinados para encontrar a magnitude vetorial absoluta:
    A = sqrt(AccX^2 + AccY^2 + AccZ^2)
2.  **Filtro:** Um filtro de média móvel com janela de 3 amostras é aplicado ao valor para suavizar ruídos mecânicos de alta frequência.
3.  **Máquina de Estados:** Um passo é validado se a magnitude filtrada ultrapassar o limiar de impacto (1.2G) e, subsequentemente, cair abaixo do limiar de repouso (0.95G).
4.  **Restrição Temporal entre Passos:** Para prevenir contagens duplas causadas pela reverberação mecânica de um único impacto físico, o algoritmo exige um intervalo mínimo obrigatório de 100 milissegundos (`tempoEntrePassos`) entre o registro de cada passo consecutivo.
5.  **Bloqueio por Rotação:** Se a magnitude do giroscópio exceder o limiar de 100.0 graus por segundo, o incremento de passos entra em um período de espera (cooldown) de 500 milissegundos para evitar a contabilização de movimentos anômalos, como o chacoalhar do animal.

---

## 5. Inteligência Artificial na Borda (Edge AI), Memória e Sincronização

O microcontrolador roda um modelo Random Forest convertido para C++ (`ModeloPatinhas.h`). A arquitetura paralela implementa estruturas nativas do RTOS para prevenir colisão de variáveis e vazamentos de memória.

### 5.1. Double Buffering e Memória Estática
A matriz bidimensional que alimenta o classificador foi duplicada (`float buffersIA[2][TAMANHO_JANELA][6]`).
*   Enquanto a Task de Rede extrai características isoladas de média e desvio padrão para classificar o estado no **Buffer 0**, a Task de Sensor continua a amostragem gravando ativamente no **Buffer 1**.
*   Esta técnica recicla posições físicas na SRAM com alocação estática. Evita-se a alocação dinâmica (`malloc`), prevenindo a fragmentação do heap (Heap Fragmentation). Ao final do processamento, o índice do array é resetado nativamente com a inversão da matriz ativa, eliminando riscos de Race Condition.

### 5.2. Mutex e Proteção de Barramento
O sensor MPU9250 e o OLED SSD1306 compartilham eletricamente o mesmo barramento I2C. O firmware utiliza um Semáforo de Exclusão Mútua (`xSemaphoreCreateMutex`). A Task que solicitar acesso aos pinos tranca a comunicação, obrigando as demais threads a aguardarem a liberação. Isso previne colisões elétricas simultâneas que causariam o travamento do microcontrolador (Kernel Panic).

### 5.3. Filas Assíncronas (Queues)
Os acionamentos de incremento de passos passam da Task do Sensor para a Task de Rede indiretamente, através de uma fila estruturada (`xQueueCreate`). A rotina do acelerômetro apenas envia o gatilho para a fila e retoma a amostragem em microssegundos, sem aguardar a latência do envio da requisição POST na rede.

---

## 6. Monitoramento de Bateria (ADC)

O sistema realiza a leitura contínua do nível de carga da bateria através do conversor analógico-digital (ADC) do microcontrolador.

### 6.1. Isolamento de Hardware (ADC1 vs ADC2)
A arquitetura do ESP32-C3 desativa fisicamente o conversor ADC2 quando o rádio Wi-Fi está em operação. Para garantir a estabilidade e a continuidade da leitura enquanto a placa transmite dados sem fio, o circuito é mapeado para o **Pino 2 (GPIO 2)**, pertencente ao **ADC1** (`ADC1_CH2`). A porta analógica é inicializada explicitamente no bloco de configuração do sistema através do comando `pinMode(PINO_BATERIA, INPUT)`.

### 6.2. Calibração Empírica e Mapeamento
A conversão do pulso elétrico para uma métrica percentual legível de 0% a 100% ocorre dentro da função `lerBateria()`. Em vez de adotar um multiplicador de tensão padrão, a arquitetura utiliza um mapeamento empírico do sinal bruto (`analogRead`):
*   **Escalonamento Flexível:** A função `map()` converte a leitura utilizando tetos paramétricos. Os limites inferior e superior refletem os valores numéricos gerados na porta analógica quando o circuito atinge seu corte mínimo de energia e sua carga máxima física.
*   **Contenção de Ruído (Clamping):** Devido à natureza oscilante do sinal analógico e à resistência variável do hardware, a porcentagem processada passa por condicionais de contenção (`if (porcentagem > 100)` e `< 0`). Essa técnica trava a variável final e impede que anomalias matemáticas sejam propagadas para o backend.

### 6.3. Desacoplamento, Média Aritmética e Otimização de Tráfego
A amostragem da carga e as requisições de rede HTTP (`sendBateriaPost`) não são executadas no mesmo ritmo da amostragem do giroscópio. A lógica está encapsulada na Task de menor prioridade e opera através de um acumulador de ciclos de inferência (`contadorBateria`). O processamento ocorre da seguinte forma:
*   O ciclo é acionado de forma intervalada, aproximadamente a cada 60 segundos.
*   Ao atingir esse marco temporal na Task de Rede, o código executa um laço rápido efetuando múltiplas amostras (ex: 10 capturas seguidas) com micropausas de estabilização de 2 milissegundos.
*   A média aritmética dessas capturas é calculada internamente para estabilizar flutuações e descartar ruídos eletromagnéticos residuais do ADC antes da transmissão HTTP.
*   Essa arquitetura garante que a leitura da bateria forneça um valor elétrico limpo, limitando o tráfego TCP desnecessário e otimizando o uso do processador, tudo isso sem prejudicar a execução rigorosa de 50Hz do sensor inercial.