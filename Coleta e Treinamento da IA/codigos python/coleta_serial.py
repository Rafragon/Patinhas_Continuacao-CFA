import serial
import time
import csv

# Instale as bibliotecas necessárias antes de rodar:
# pip install pyserial

# ================= CONFIGURAÇÕES =================
# Substitua pela porta do seu ESP32 (ex: 'COM3' no Windows, '/dev/ttyUSB0' no Linux/Mac)
PORTA_SERIAL = 'COM3'
BAUD_RATE = 115200
NOME_DO_ARQUIVO = 'dados_caminhando_1.csv'
TEMPO_DE_COLETA = 30  # Segundos que você quer gravar

# =================================================

print(f"Tentando conectar na porta {PORTA_SERIAL}...")

try:
    # Conecta com a placa
    esp32 = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
    time.sleep(2)  # Espera a conexão estabilizar

    print("Conectado! Reinicie o ESP32 (aperte o botão RST/EN ou desconecte e conecte o cabo).")
    print(f"Irei gravar os dados por {TEMPO_DE_COLETA} segundos...")

    dados = []
    tempo_inicial = None

    with open(NOME_DO_ARQUIVO, mode='w', newline='') as arquivo_csv:
        escritor = csv.writer(arquivo_csv)

        while True:
            # Lê a linha que vem do ESP32
            linha_bruta = esp32.readline().decode('utf-8').strip()

            if linha_bruta:
                # Se for a mensagem de calibração, apenas avisa na tela
                if "Calibrando" in linha_bruta or "Calibracao" in linha_bruta or "Modo" in linha_bruta:
                    print(f"Status do ESP32: {linha_bruta}")
                    continue

                # Se for o cabeçalho, escreve no arquivo e marca o tempo inicial
                if "accX" in linha_bruta:
                    escritor.writerow(linha_bruta.split(','))
                    print("Cabeçalho recebido. Começando a gravar os dados reais!")
                    tempo_inicial = time.time()
                    continue

                # Se já recebemos o cabeçalho, começamos a salvar os dados
                if tempo_inicial is not None:
                    # Divide os números pela vírgula e salva no arquivo
                    valores = linha_bruta.split(',')
                    escritor.writerow(valores)

                    tempo_passado = time.time() - tempo_inicial

                    # Para a coleta após o tempo estipulado
                    if tempo_passado >= TEMPO_DE_COLETA:
                        print(
                            f"\nSucesso! Coleta de {TEMPO_DE_COLETA} segundos finalizada.")
                        print(
                            f"Arquivo '{NOME_DO_ARQUIVO}' salvo na mesma pasta deste script.")
                        break

except serial.SerialException as e:
    print(
        f"Erro na porta serial: {e}. O Monitor Serial do Arduino está fechado? A porta está certa?")
finally:
    if 'esp32' in locals() and esp32.is_open:
        esp32.close()
