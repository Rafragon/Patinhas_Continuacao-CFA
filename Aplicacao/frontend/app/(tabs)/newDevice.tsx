import React, { useState } from 'react';
import { View, Linking, Text, Button, StyleSheet, SafeAreaView, Platform } from 'react-native';
import { WebView } from 'react-native-webview';
import Header from '@/patinhas-components/Header'; // Ajuste o caminho se necessário
import useServerIP from '@/patinhas-components/useServerIP';

export default function NewDevice() {
    const [conectadoNaColeira, setConectadoNaColeira] = useState(false);
    const ipServidor = useServerIP();
    // Renderiza o WebView com a página do ESP32
    if (conectadoNaColeira) {
        return (
            <SafeAreaView style={{ flex: 1, backgroundColor: '#2c3e50' }}>
                <Header />
                <View style={styles.webviewHeader}>
                    <Button 
                        title="Voltar" 
                        color="#e74c3c" 
                        onPress={() => setConectadoNaColeira(false)} 
                    />
                    <Text style={styles.webviewTitle}>Configuração Patinhas</Text>
                    <View style={{ width: 60 }} /> {/* Espaçador para centralizar título */}
                </View>
                
                {Platform.OS === 'web' ? (
                    <iframe 
                        src="http://192.168.4.1" 
                        style={{ width: '100%', height: '100%', border: 'none' }} 
                        title="Configuração Patinhas"
                    />
                ) : (
                    <WebView 
                        source={{ uri: 'http://192.168.4.1' }} 
                        style={{ flex: 1 }}
                        startInLoadingState={true}
                        renderLoading={() => (
                            <View style={styles.loadingContainer}>
                                <Text style={styles.loadingText}>Buscando coleira...</Text>
                                <Text style={styles.loadingSub}>Certifique-se de que está conectado na rede "Patinhas-Config"</Text>
                            </View>
                        )}
                    />
                )}
            </SafeAreaView>
        );
    }

    // Tela de instruções (antes de carregar o WebView)
    return (
        <SafeAreaView style={{ flex: 1, backgroundColor: '#1a1a1a' }}>
            <Header />
            
            <View style={styles.instrucoesContainer}>
                <Text style={styles.tituloInstrucoes}>Nova Coleira Patinhas</Text>
                
                <View style={styles.cardPassos}>
                    <Text style={styles.passo}>1. Ligue a coleira e aguarde a tela exibir "Buscando Wifi...".</Text>
                    <Text style={styles.passo}>2. Saia do aplicativo e abra as <Text style={{fontWeight: 'bold'}}>Configurações de Wi-Fi</Text> do seu celular.</Text>
                    <Text style={styles.passo}>3. Conecte-se à rede chamada <Text style={{color: '#f1c40f', fontWeight: 'bold'}}>"Patinhas-Config"</Text>.</Text>
                    <Text style={styles.passo}>4. Volte para este aplicativo e clique no botão abaixo.</Text>
                </View>

                <View style={styles.botaoContainer}>
                    <View style={{ marginBottom: 15 }}>
                        <Button
                            title="Já estou conectado na rede wi-fi"
                            color="#27ae60"
                            onPress={() => setConectadoNaColeira(true)}
                        />
                    </View>
                    <View style={{ marginBottom: 15 }}>
                        <Button
                            title="Redefinir Wi-Fi da Coleira"
                            color="red"
                            onPress={async () => {
                                try {
                                    // Utiliza a variável ipServidor extraída no topo do componente
                                    const respostaServidor = await fetch(`http://${ipServidor}:8080/ipColeira`);
                                    const dadosServidor = await respostaServidor.json();

                                    if (dadosServidor.ip) {
                                        await fetch(`http://${dadosServidor.ip}/reset`);
                                        alert("Coleira reiniciada. Conecte-se na rede Patinhas-Config.");
                                    } else {
                                        alert("O IP da coleira ainda não foi registrado no servidor.");
                                    }
                                } catch (error) {
                                    alert("Erro ao tentar redefinir a rede. Verifique se a coleira está ligada.");
                                }
                            }}
                        />
                    </View>
                    <View style={{ marginBottom: 15 }}>
                        <Button
                            title="Abrir Configurações de Wi-Fi"
                            color="#3498db"
                            onPress={() => {
                                if (Platform.OS === 'ios') {
                                    Linking.openURL('App-Prefs:root=WIFI');
                                } else if (Platform.OS === 'android') {
                                    Linking.sendIntent('android.settings.WIFI_SETTINGS');
                                }
                            }}
                        />
                    </View>
                </View>
            </View>
        </SafeAreaView>
    );
}

const styles = StyleSheet.create({
    instrucoesContainer: {
        flex: 1,
        padding: 20,
        alignItems: 'center',
        justifyContent: 'center',
    },
    tituloInstrucoes: {
        fontSize: 24,
        fontFamily: 'Montserrat_700Bold',
        color: '#ffffff',
        marginBottom: 30,
    },
    cardPassos: {
        backgroundColor: '#2c3e50',
        padding: 20,
        borderRadius: 15,
        width: '100%',
        marginBottom: 30,
    },
    passo: {
        fontSize: 16,
        fontFamily: 'Montserrat_400Regular',
        color: '#ecf0f1',
        marginBottom: 15,
        lineHeight: 24,
    },
    botaoContainer: {
        width: '100%',
        paddingHorizontal: 10,
    },
    webviewHeader: {
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'space-between',
        backgroundColor: '#1a1a1a',
        paddingHorizontal: 10,
        paddingVertical: 10,
    },
    webviewTitle: {
        color: 'white',
        fontFamily: 'Montserrat_700Bold',
        fontSize: 16,
    },
    loadingContainer: {
        position: 'absolute',
        top: 0,
        bottom: 0,
        left: 0,
        right: 0,
        backgroundColor: '#2c3e50',
        alignItems: 'center',
        justifyContent: 'center',
    },
    loadingText: {
        color: '#f1c40f',
        fontSize: 20,
        fontFamily: 'Montserrat_700Bold',
        marginBottom: 10,
    },
    loadingSub: {
        color: '#ecf0f1',
        fontFamily: 'Montserrat_400Regular',
        textAlign: 'center',
        paddingHorizontal: 40,
    }
});