# Detalhamento do Frontend (React Native Web)

O código-fonte atua como o painel de controle do tutor. Construído com o ecossistema React Native em conjunto com o framework Expo, o projeto está atualmente sendo testado e executado em sua versão Web (website), embora possua arquitetura híbrida compatível com futura compilação móvel.

## 1. Arquitetura e Configurações Base

O aplicativo segue uma arquitetura modular orientada a componentes funcionais.

*   **Roteamento (Navegação):** O sistema de rotas é gerenciado pelo **Expo Router**. A estrutura principal foi organizada através de abas (Tabs), adaptadas para a visualização no navegador.
*   **Compatibilidade Web:** O projeto utiliza a biblioteca `react-native-web` para traduzir os componentes nativos para elementos DOM (HTML/CSS). O arquivo de configuração do Expo define o output da versão web como estático. A identidade visual padroniza as fontes da família `Montserrat`.

## 2. Comunicação de Rede e Polling

A arquitetura de obtenção de dados utiliza um modelo de Polling.

*   **Resolução de IP:** O hook customizado isola a lógica de ambiente. Na execução web/desktop atual, ele identifica o sistema operacional e retorna `127.0.0.1` para conectar-se ao servidor Flask rodando localmente na máquina.
*   **Sincronização Contínua:** O componente de status executa a instrução `setInterval` para disparar conexões HTTP de leitura a cada 2000 milissegundos (2 segundos). Isso atualiza simultaneamente a volumetria de passos e a classificação da inteligência artificial.

## 3. Lógica de Negócio e Heurísticas

Regras matemáticas de uso veterinário estão encapsuladas no módulo do Estimador.

*   **Fórmula do Passo Ideal:** Calcula o gasto energético necessário com base no peso e porte do cachorro, aplicando multiplicadores condicionais (nível de energia, idade, ambiente, saúde). Devolve um alvo customizado com margem de segurança de ±15%.
*   **Análise Estatística:** Agrupa o histórico JSON enviado pelo servidor e calcula a média de movimentação, com filtragem retroativa por dias específicos.

## 4. Visualização de Dados e Telas Principais

A interface utiliza componentes gráficos da biblioteca `react-native-chart-kit` renderizados no navegador.

*   **Dashboard em Tempo Real:** Renderiza informações estáticas do pet associadas à barra de progressão diária. O status de ação detectado pela IA é exibido em destaque.
*   **Análise Histórica de Passos:** Monta um gráfico de barras (`BarChart`) com os dados quantitativos dos últimos 5 dias.
*   **Monitoramento Comportamental (IA):** Visualiza as inferências em um gráfico de barras empilhadas (`StackedBarChart`), exibindo a proporção percentual diária dos estados ("Correndo", "Andando", "Parado", "Coçando").
*   **Restrição de Provisionamento IoT (Atenção Web):** A tela dedicada ao pareamento Bluetooth utiliza a biblioteca nativa `react-native-ble-plx`. Durante a fase de testes em ambiente Web, este módulo não possui acesso direto ao rádio Bluetooth sem o uso específico da Web Bluetooth API, limitando seu funcionamento estritamente ao layout no navegador.
