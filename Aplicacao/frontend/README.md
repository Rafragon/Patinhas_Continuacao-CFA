```markdown
# Detalhamento do Frontend (React Native - Web e Mobile)

O código-fonte atua como o painel de controle do tutor. Construído com o ecossistema React Native em conjunto com o framework Expo, o projeto possui arquitetura híbrida compatível com execução no navegador (Web) e futura compilação móvel nativa.

## 1. Arquitetura e Configurações Base

O aplicativo segue uma arquitetura modular orientada a componentes funcionais.

*   **Roteamento (Navegação):** O sistema de rotas é gerenciado pelo **Expo Router**. A estrutura principal foi organizada através de abas (Tabs), adaptadas tanto para visualização em telas sensíveis ao toque quanto no navegador.
*   **Compatibilidade Híbrida:** O projeto utiliza a biblioteca `react-native-web` para traduzir os componentes nativos para elementos DOM (HTML/CSS) durante a execução web. A identidade visual padroniza as fontes da família `Montserrat`.

## 2. Comunicação de Rede e Polling

A arquitetura de obtenção de dados utiliza um modelo de Polling.

*   **Resolução de IP:** O hook customizado `useServerIP` isola a lógica de ambiente. Na execução web/desktop no mesmo ambiente do backend, ele retorna `127.0.0.1` para conectar-se ao servidor Flask local. Na execução móvel, ele deve ser configurado para retornar o IP estático local da rede (ex: `192.168.0.X`).
*   **Sincronização Contínua:** O componente de status executa a instrução `setInterval` para disparar conexões HTTP de leitura a cada 2000 milissegundos (2 segundos). Isso atualiza simultaneamente a volumetria de passos e a classificação da inteligência artificial. Em paralelo, o sistema solicita e atualiza o nível percentual de carga da bateria do dispositivo de hardware.

## 3. Lógica de Negócio e Heurísticas

Regras matemáticas de uso veterinário estão encapsuladas no módulo do Estimador.

*   **Fórmula do Passo Ideal:** Calcula o gasto energético necessário com base no peso e porte do cachorro, aplicando multiplicadores condicionais (nível de energia, idade, ambiente, saúde). Devolve um alvo customizado com margem de segurança de ±15%.
*   **Análise Estatística:** Agrupa o histórico JSON enviado pelo servidor e calcula a média de movimentação, com filtragem retroativa por dias específicos.

## 4. Visualização de Dados e Telas Principais

A interface utiliza componentes gráficos da biblioteca `react-native-chart-kit`.

*   **Dashboard em Tempo Real:** Renderiza informações estáticas do pet associadas à barra de progressão diária. O status de ação detectado pela IA é exibido em destaque, juntamente com o indicador visual da porcentagem de bateria recebida pelo servidor.
*   **Análise Histórica de Passos:** Monta um gráfico de barras (`BarChart`) com os dados quantitativos dos últimos 5 dias.
*   **Monitoramento Comportamental (IA):** Visualiza as inferências em um gráfico de barras empilhadas (`StackedBarChart`), exibindo a proporção percentual diária dos estados ("Correndo", "Andando", "Parado", "Coçando").
*   **Provisionamento IoT (Portal Cativo e Wi-Fi):** A interface de configuração de rede acessa o modo Access Point gerado pelo `WiFiManager` do ESP32. 
    *   **Mobile:** O aplicativo oferece redirecionamento nativo via `Linking` para as configurações de rede do celular e um botão para instanciar um `WebView` apontado para o endereço local `http://192.168.4.1`. 
    *   **Web:** A tela exibe instruções textuais diretas guiando o usuário a conectar o computador à rede `Patinhas-Config` e abrir o IP de provisionamento no navegador.
