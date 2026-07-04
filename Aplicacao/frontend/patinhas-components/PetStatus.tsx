import React, { useEffect, useState } from "react";
import { Image, ScrollView, StyleSheet, Text, View } from "react-native";
import useServerIP from "./useServerIP";
import { Ionicons } from "@expo/vector-icons";
import {calcularMediaPassos, estimarPassosDiarios} from "./Estimador";

const PetStatus = (pet:any) => {
  const [passos, setPassos] = useState(0);
  const [estadoAtual, setEstadoAtual] = useState("A carregar..."); // Novo estado para a IA

  const hojeBRT = new Date(new Date().getTime() - 3 * 60 * 60 * 1000).toISOString().split("T")[0];

  useEffect(() => {
    const getDadosPet = async () => {
      try {
        const timestamp = new Date().getTime(); 
        const ip = useServerIP();
        
        // 1. Consulta os passos
        const responsePassos = await fetch("http://" + ip + ":8080/passosDia?dia=" + hojeBRT + "&t=" + timestamp);
        const dataPassos = await responsePassos.json();
        
        if(dataPassos.num_passos !== undefined) {
             setPassos(prevPassos => {
                 if (prevPassos === dataPassos.num_passos) return prevPassos;
                 return dataPassos.num_passos;
             });
        }

        // 2. Consulta o estado comportamental da IA
        const responseEstado = await fetch("http://" + ip + ":8080/estado?t=" + timestamp);
        const dataEstado = await responseEstado.json();

        if(dataEstado.estado !== undefined) {
            setEstadoAtual(prevEstado => {
                if (prevEstado === dataEstado.estado) return prevEstado;
                return dataEstado.estado;
            });
        }

      } catch (error) {
        console.error("Erro ao buscar dados do servidor:", error);
      }
    };

    getDadosPet();

    const intervalo = setInterval(() => {
        getDadosPet();
    }, 2000);

    return () => clearInterval(intervalo);

  }, []); 
  
  const estimativa = estimarPassosDiarios(pet);
  const passosIdeal = estimativa.estimate;

  return (
    <ScrollView contentContainerStyle={styles.container}>
      
      {
        Number(passos) < 100 ? 
          <Text style={[styles.nomeMid, {fontFamily: 'Montserrat_700Bold'}]}>{pet.pet.nome} precisa de atenção!</Text> : 
          <Text style={[styles.nomeHigh, {fontFamily: 'Montserrat_700Bold'}]}>{pet.pet.nome} está indo muito bem!</Text>
      }

      <Text style={[styles.subtitulo, { fontFamily: 'Montserrat_400Regular' }]}>
        “Brinque um pouco comigo”{"\n"}
        <Text style={{ fontWeight: "300" }}>Opa! Quem disse isso?</Text>
      </Text>

      <Image
        source={{ uri: pet.pet.fotoSrc }}
        style={styles.foto}
      />

      <View style={[styles.progressBar, {borderWidth: 2, borderColor: '#c0c0c0ff'}]}>
        <View style={[Number(passos) < 100 ? styles.progressFillMid : styles.progressFillHigh, { width: `${(Number(passos) / 500) * 100}%` }]} />
      </View>

      <Text style={[styles.passos, { fontFamily: 'Montserrat_400Regular' }]}>{passos} passos</Text>
      
      {/* Nova área de exibição da IA */}
      <View style={styles.estadoContainer}>
        <Text style={[styles.estadoTexto, { fontFamily: 'Montserrat_700Bold' }]}>
          Estado atual: {estadoAtual.toUpperCase()}
        </Text>
      </View>

      <View style={styles.infoBox}>
        <Text style={styles.title}>Dados do seu pet</Text>

        <View style={styles.editIconContainer}>
          <Ionicons name="pencil" size={20} color="#fff" />
        </View>
        
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular', fontWeight: '600'}]}>Média de passos do {pet.pet.nome}: {calcularMediaPassos(pet, null).toFixed(1)} </Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular', fontWeight: '600'}]}>Ideal de passos por dia: {passosIdeal} </Text>
        
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>{"\n"}Nome: {pet.pet.nome}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Peso (kg): {pet.pet.pesoKg}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Porte: {pet.pet.porte}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Idade (anos): {pet.pet.idadeAnos}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Energia: {pet.pet.energia}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Castrado: {pet.pet.castrado ? 'sim' : 'não'}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Tem problema de saude: {pet.pet.temProblemaSaude ? 'sim' : 'não'}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Objetivo: {pet.pet.objetivo}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Ambiente ruim: {pet.pet.ambienteRuim}</Text>
        <Text style={[styles.infoText, {fontFamily: 'Montserrat_400Regular'}]}>Cor: {pet.pet.cor}</Text>

        <Text style={[{fontFamily: 'Montserrat_400Regular', color:'white', fontSize:13}]}>{"\n"} * o ideal de passos é calculado com base nas características do seu pet!</Text>
      </View>

    </ScrollView>
  );
};

const styles = StyleSheet.create({
  container: {
    alignItems: "center",
    paddingVertical: 40,
  },

  nomeMid: {
    fontSize: 30,
    fontWeight: '700',
    color: "#d9e021",
    textAlign: "center",
    marginBottom: 6,
    paddingHorizontal: 40,
  },

  nomeHigh: {
    fontSize: 26,
    fontWeight: "700",
    color: "#57c448",
    textAlign: "center",
    marginBottom: 6,
    paddingHorizontal: 40,
  },

  subtitulo: {
    fontSize: 16,
    color: "#999999ff",
    textAlign: "center",
    marginBottom: 20,
  },

  foto: {
    width: 280,
    height: 280,
    borderRadius: 160,
    marginBottom: 20,
  },

  progressBar: {
    width: 260,
    height: 16,
    backgroundColor: "#EFEFEF",
    borderRadius: 8,
    overflow: "hidden",
    marginBottom: 16,
  },

  progressFillMid: {
    height: "100%",
    backgroundColor: "#E3EB3A",
    borderRadius: 6,
  },

  progressFillLow: {
    height: "100%",
    backgroundColor: "red",
    borderRadius: 6,
  },

  progressFillHigh: {
    height: "100%",
    backgroundColor: "#57c448",
    borderRadius: 8,
  },

  passos: {
    fontSize: 36,
    fontWeight: "900",
    color: "#000",
    marginBottom: 4,
  },

  estadoContainer: {
    backgroundColor: "#2c3e50",
    paddingVertical: 8,
    paddingHorizontal: 20,
    borderRadius: 20,
    marginBottom: 16,
  },

  estadoTexto: {
    color: "#ffffff",
    fontSize: 18,
  },

  infoBox: {
    backgroundColor: 'rgba(209, 209, 209, 0.82)',
    padding: 16,
    borderRadius: 16,
    marginVertical: 20,
    width: '90%',
    alignSelf: 'center',
    shadowColor: '#000',
    shadowOpacity: 0.25,
    shadowOffset: { width: 0, height: 4 },
    shadowRadius: 8,
    elevation: 6, 
  },

  infoText: {
    color: '#242424ff',
    fontSize: 16,
    marginVertical: 2,
    lineHeight: 22,
  },

  title: {
    fontSize: 22,
    fontWeight: '600',
    color: '#585858ff',
    marginBottom: 10,
    textAlign: 'center',
  },

  editIconContainer: {
    position: 'absolute',
    top: 12,
    right: 12,
  }
});

export default PetStatus;