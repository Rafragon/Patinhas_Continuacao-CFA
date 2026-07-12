# Detalhamento Backend Servidor 

O código apresenta uma API desenvolvida em Python com o framework Flask. Ele atua como servidor central para gerenciar a autodescoberta de dispositivos e persistir os dados de telemetria enviados pelo hardware.

## 1. Gerenciamento de Rede e Concorrência

O sistema opera com dois serviços de rede simultâneos utilizando a biblioteca `threading`.

*   **Thread Principal (TCP/HTTP):** Executa o servidor web Flask na porta 8080, configurado para escutar requisições externas em todas as interfaces de rede por meio do host `0.0.0.0`. O suporte a requisições de origens cruzadas (CORS) está habilitado globalmente.
*   **Thread Secundária (UDP):** Executa a função `broadcast_listener` como um daemon (processo em segundo plano) para não bloquear a thread principal. O socket escuta requisições na porta 50000 e responde com o endereço IP local do servidor sempre que intercepta a string `DISCOVER_SERVER`.
*   **Resolução de IP:** O endereço IP local é identificado ativamente pela função `get_local_ip()`, que simula uma conexão de soquete datagrama com o DNS público do Google (8.8.8.8).

## 2. Persistência de Dados

A persistência atual dispensa sistemas de gerenciamento de banco de dados (SGBDs), operando diretamente na memória física.

*   O sistema utiliza manipulação de arquivos estáticos através das funções `carregar_dados()` e `salvar_dados()`.
*   A cada requisição de escrita, o código lê integralmente o arquivo `animais.json`, modifica os dados em memória e o reescreve no disco.
*   O salvamento é formatado com `indent=2` e `ensure_ascii=False`, preservando caracteres especiais e estruturando a identação visual do documento JSON.

## 3. Mapeamento de Rotas (Endpoints)

As rotas RESTful integram a comunicação entre o microcontrolador ESP32 (entradas) e o aplicativo móvel (saídas).

*   **`POST /` (Processamento de Passos):** Recebe valores numéricos em formato de texto (`text/plain`). O algoritmo identifica a data do sistema operacional (`date.today()`) e procura por um registro correspondente dentro da chave de passos do primeiro animal no JSON. Se a data for encontrada, soma os novos passos ao valor existente; caso contrário, cria uma nova entrada para o dia.
*   **`GET /`:** Retorna em formato numérico simples o total mantido em cache pela variável global `totalPassos` através da função `ultimo_passo()`.
*   **`POST /estado` (Atualização de IA):** Capta o rótulo de inferência do modelo Random Forest recebido via texto e subscreve o campo `estadoAtual` pertencente ao primeiro pet da base de dados.
*   **`GET /estado`:** Expõe o último comportamento classificado (como "andando" ou "parado") e implementa o valor `desconhecido` como retorno padrão caso o campo esteja vazio ou o arquivo acabe de ser criado.
*   **`GET /animalInfo`:** Consome o arquivo JSON e devolve a lista integral presente sob o array `pets` em formato serializado. Retorna erro HTTP 400 se a base estiver vazia.
*   **`GET /passosDia`:** Requer a submissão do argumento `dia` na URL da requisição. O código itera sobre o histórico do arquivo procurando correspondência exata. Se encontrar, retorna a quantidade de passos daquela data; se falhar, retorna um erro HTTP 404 com uma mensagem informando a ausência do registro.
