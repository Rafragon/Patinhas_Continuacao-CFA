import socket
import time
import csv

# ================= CONFIGURAÇÕES =================
# <--- COLOQUE O IP QUE APARECE NO MONITOR SERIAL
IP_ESP32 = '192.168.15.120'
PORTA = 80
NOME_DO_ARQUIVO = 'dados_velocimetro_wifi.csv'
TEMPO_DE_COLETA = 30
# =================================================

print(f"Tentando conectar ao ESP32 em {IP_ESP32}:{PORTA}...")

# Criamos a variável do socket como None para garantir o fechamento no 'finally'
sock = None

try:
    # Cria um socket TCP/IP
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)  # Timeout de 5 segundos para conectar
    sock.connect((IP_ESP32, PORTA))

    # Criamos um arquivo para salvar os dados
    with open(NOME_DO_ARQUIVO, mode='w', newline='') as arquivo_csv:
        escritor = csv.writer(arquivo_csv)
        tempo_inicial = None
        buffer = ""  # Buffer para acumular fragmentos de texto
        coleta_finalizada = False  # Variável de controle para encerrar o loop externo

        print("Conectado! Aguardando dados...")

        # Loop de coleta (agora controlado pela variável)
        while not coleta_finalizada:
            # Recebe os dados brutos via Wi-Fi
            dados_brutos = sock.recv(1024).decode('utf-8')
            if not dados_brutos:
                print("ESP32 encerrou a conexão.")
                break

            buffer += dados_brutos

            # Processa linha por linha caso o buffer tenha recebido mais de uma
            while "\n" in buffer:
                linha, buffer = buffer.split("\n", 1)
                linha = linha.strip()

                if not linha:
                    continue

                # Identifica o cabeçalho e inicia o cronômetro
                if "accX" in linha:
                    escritor.writerow(linha.split(','))
                    print("Cabeçalho recebido! Gravando dados...")
                    tempo_inicial = time.time()
                    continue

                # Grava os dados numéricos
                if tempo_inicial is not None:
                    escritor.writerow(linha.split(','))

                    # Verifica se o tempo acabou
                    if time.time() - tempo_inicial >= TEMPO_DE_COLETA:
                        print(
                            f"\nColeta de {TEMPO_DE_COLETA}s finalizada com sucesso!")
                        coleta_finalizada = True  # Ativa o fim do loop externo
                        break  # Sai do loop interno (\n)

except Exception as e:
    print(f"Erro na conexão Wi-Fi: {e}")
finally:
    # O bloco 'finally' sempre roda, garantindo que o socket seja fechado da forma correta
    if sock:
        sock.close()
        print("Socket de rede fechado.")
    print(f"Arquivo '{NOME_DO_ARQUIVO}' fechado.")
