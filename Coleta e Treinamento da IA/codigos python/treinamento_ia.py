import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from micromlgen import port
import os

# Configurações
TAMANHO_JANELA = 100  # 100 amostras a 50Hz = 2 segundos
PASTA_DADOS = 'dados'

ARQUIVOS = {
    'parado': os.path.join(PASTA_DADOS, 'dados_parado.csv'),
    'andando': os.path.join(PASTA_DADOS, 'dados_andando.csv'),
    'correndo': os.path.join(PASTA_DADOS, 'dados_correndo.csv'),
    'se_cocando': os.path.join(PASTA_DADOS, 'dados_se_cocando.csv')
}

X = []
y = []
mapa_classes = {nome: idx for idx, nome in enumerate(ARQUIVOS.keys())}


def extrair_caracteristicas(janela):
    # Calcula a média e o desvio padrão (vibração) para os 6 eixos
    medias = np.mean(janela, axis=0)
    desvios = np.std(janela, axis=0)
    # Retorna um vetor único com 12 valores que resumem os 2 segundos
    return np.concatenate((medias, desvios))


print("Processando arquivos CSV...")
for classe, arquivo in ARQUIVOS.items():
    if not os.path.exists(arquivo):
        print(f"Arquivo ignorado (não encontrado): {arquivo}")
        continue

    df = pd.read_csv(arquivo)

    # Extrai apenas as colunas numéricas dos sensores
    dados = df[['accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ']].values

    # Segmenta os dados em janelas de 2 segundos
    for i in range(0, len(dados) - TAMANHO_JANELA, TAMANHO_JANELA):
        janela = dados[i: i + TAMANHO_JANELA]
        features = extrair_caracteristicas(janela)

        X.append(features)
        y.append(mapa_classes[classe])

X = np.array(X)
y = np.array(y)

print(f"Total de janelas processadas: {len(X)}")

print("Treinando o modelo Random Forest...")
modelo = RandomForestClassifier(n_estimators=30, max_depth=10, random_state=42)
modelo.fit(X, y)

acuracia = modelo.score(X, y)
print(f"Acurácia do treinamento: {acuracia * 100:.2f}%")

print("Convertendo modelo para C++...")
# A biblioteca micromlgen traduz o modelo Python para C++ puro
codigo_cpp = port(modelo, classmap={
                  idx: nome for idx, nome in enumerate(ARQUIVOS.keys())})

with open('ModeloPatinhas.h', 'w') as f:
    f.write(codigo_cpp)

print("Finalizado. Arquivo 'ModeloPatinhas.h' gerado na pasta atual.")
