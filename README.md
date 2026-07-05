# Funcionamento Técnico do Dispositivo - Patinhas-CFA

Este documento detalha o funcionamento interno, a eletrônica, os algoritmos embarcados e os protocolos de comunicação do dispositivo IoT (coleira inteligente). Esta implementação é uma evolução direta do [Patinhas](https://github.com/willianjsf/Patinhas-CFA/tree/main), estendendo as funcionalidades de hardware e contagem de passos com modelos de Machine Learning para classificação comportamental na borda.
---

## 1. Arquitetura de Hardware e Conexões

O dispositivo físico foi projetado com um design minimalista, focado em leveza e reaproveitamento de materiais. Ele é construído com base nos seguintes componentes e reciclados:

| Componente | Especificação/Origem | Função no Sistema |
| --- | --- | --- |
| Microcontrolador | ESP32-C3 Super Mini | Processamento core, inferência em tempo real, comunicação TCP/UDP Wi-Fi. |
| Sensor Inercial | MPU9250 | Captura vetorial de Aceleração e Giroscópio via protocolo I2C. |
| Display Periférico | OLED SSD1306 0.42'' | Interface de debug do hardware, status de conexão AP/IP e contagem local. |
| Alimentação | Bateria Li-po 3.7V 300mAh | Fornecer autonomia de energia ao ESP32. (Reciclada de carregador de fones bluetooth). |
| Gerenciador de Carga | Módulo Li-po genérico | Proteção e recarregamento seguro da bateria via USB. (Reciclado de carregador de fones bluetooth). |
| Invólucro | Case plástica com velcro | Proteção do circuito, isolamento elétrico e acoplamento seguro na coleira do pet. |
| Placa Base | Placa de circuito universal | Soldagem estrutural com pinos fêmea (permitindo a troca e reutilização do microcontrolador e sensores). |

### Barramento de Comunicação Interna (I2C)
A comunicação entre o ESP32-C3, o sensor IMU e o display OLED é realizada através do barramento físico I2C soldado na placa universal. Os pinos lógicos mapeados são:
* **SDA (Dados):** Pino GPIO 5
* **SCL (Clock):** Pino GPIO 6
* **Endereço do MPU9250:** 0x68 (padrão I2C)

---

## 2. Inicialização e Conexão de Rede (Protocolos)

O dispositivo executa uma rotina de inicialização em camadas para garantir a conexão sem fios e a localização do servidor local sem a necessidade de definir credenciais estáticas no código.

### 2.1. Provisionamento de Wi-Fi (WiFiManager)
1. Ao ligar, o firmware lê a memória não-volátil (NVS) do ESP32 para buscar a última rede Wi-Fi salva.
2. Caso não encontre ou não consiga conectar, o dispositivo entra em modo Access Point (AP).
3. Uma rede Wi-Fi aberta chamada `Patinhas-Config` (ou `Patinhas-Coleta` no script de calibração) é criada.
4. O usuário conecta-se a essa rede via smartphone, abre o portal cativo, seleciona a rede local e insere a senha correspondente.
5. As credenciais são gravadas na memória flash e a placa reinicia automaticamente para se conectar à rede configurada.

### 2.2. Autodescoberta do Servidor (UDP Broadcast)
Após estabelecer a conexão Wi-Fi, o dispositivo precisa localizar o IP do computador que executa o backend Flask.
1. O ESP32 abre um socket UDP e escuta/envia mensagens na porta `50000`.
2. A função `broadcastProcuraServidor()` dispara pacotes em broadcast para a rede local.
3. O servidor Python intercepta o pacote UDP e responde diretamente para o ESP32 confirmando seu endereço IP e a porta ativa (`8080`).
4. O dispositivo armazena esse IP em memória e encerra a busca, liberando o canal para as requisições principais.

---

## 3. Processamento de Sinais na Borda (Edge Computing)

Para otimizar o consumo de energia e a largura de banda da rede local, o dispositivo processa os dados brutos internamente em tempo real, enviando apenas os resultados consolidados.

### 3.1. Aquisição de Dados (Temporização Rígida)
O sensor MPU9250 é configurado para operar com uma escala de aceleração de ±2G e giroscópio de ±250 DPS. A leitura dos registros é controlada por uma estrutura de tempo baseada na função `millis()`, forçando uma taxa de amostragem cravada em **50 Hz** (1 leitura a cada 20 milissegundos).

### 3.2. Heurística do Contador de Passos
A contagem de passos funciona por meio de uma máquina de estados matemática:
1. **Magnitude Escalar:** Calcula a aceleração resultante combinando os três eixos espaciais:  
   $A = \sqrt{accX^2 + accY^2 + accZ^2}$
2. **Filtragem de Ruído:** Passa a magnitude por um filtro de média móvel com profundidade de 3 amostras para mitigar o ruído de alta frequência (jitter).
3. **Detecção de Picos e Vales:** Um passo é validado quando a aceleração filtrada ultrapassa o limiar superior de `1.2G` (força de impacto do membro no solo) e, logo em seguida, cruza abaixo do limiar inferior de `0.95G` (fase de suspensão/acomodação).
4. **Filtro Anti-Falso-Positivo:** O giroscópio monitora rotações bruscas. Se a velocidade angular de qualquer eixo exceder 100 graus por segundo, o algoritmo interpreta o movimento como um chacoalhar de cabeça do animal ou rotação do corpo, bloqueando o incremento do passo.

### 3.3. Classificação Comportamental (Florestas Aleatórias)
1. **Janela Temporal (Windowing):** O firmware armazena as leituras em blocos sequenciais de 100 amostras (o que corresponde a exatamente 2 segundos de movimento contínuo).
2. **Extração Estatística (Features):** Ao fechar a janela de 100 amostras, o código calcula a Média Aritmética e o Desvio Padrão para cada um dos 6 eixos (accX, accY, accZ, gyroX, gyroY, gyroZ), gerando um vetor de 12 características.
3. **Inferência Local:** O vetor é submetido à árvore de decisão gerada pelo `ModeloPatinhas.h` (compilado via `micromlgen`). O modelo classifica o comportamento nos rótulos definidos (como `parado` ou `andando`).

---

## 4. Estratégia de Envio de Dados

O dispositivo utiliza duas abordagens de transmissão HTTP dependendo do firmware executado:

### 4.1. Firmware de Operação Principal (Produção)
Funciona de forma orientada a eventos usando o protocolo **HTTP POST**:
* **Envio de Passos:** Toda vez que a heurística valida um passo físico, o ESP32 faz uma chamada rápida para `POST http://[IP_SERVIDOR]:8080/` enviando o incremento no corpo da requisição.
* **Envio de Estado:** A cada 2 segundos, após o término do processamento da janela de Machine Learning, o resultado da classificação é enviado via `POST http://[IP_SERVIDOR]:8080/estado`.

### 4.2. Firmware de Coleta de Dados (Treinamento)
Funciona de forma contínua usando uma arquitetura baseada em **Sockets TCP puros**:
1. O ESP32 abre um servidor TCP focado na porta `80`.
2. O script Python de coleta conecta-se diretamente ao endereço IP do dispositivo.
3. Uma vez estabelecido o handshake, o ESP32 envia continuamente strings formatadas em formato CSV (`accX,accY,accZ,gyroX,gyroY,gyroZ\n`) respeitando rigorosamente o intervalo de 20ms (50Hz), permitindo que o computador armazene o histórico bruto para retreinar o modelo de Inteligência Artificial.

## 5.Referências Externas

* **Estudo de base (Passometria veterinária):** [Use of pedometers to measure physical activity in dogs](https://avmajournals.avma.org/view/journals/javma/226/12/javma.2005.226.2010.xml?tab_body=pdf)
* **Documentação do Microcontrolador:** [ESP32-C3 0.42 OLED - Kevin's Blog](https://emalliab.wordpress.com/2025/02/12/esp32-c3-0-42-oled/)
* **Biblioteca do Sensor (C++):** [MPU9250 por hideakitai](https://github.com/hideakitai/MPU9250)
* **Biblioteca do Display (C++):** [U8g2lib por olikraus](https://github.com/olikraus/u8g2)
* **Repositório da Disciplina Original:** [CFA - Prof. Fábio Nakano](https://github.com/FNakano/CFA)
