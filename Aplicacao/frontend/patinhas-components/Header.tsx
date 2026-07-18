import React, { useEffect, useState } from "react";
import { View, Image, Text } from "react-native";
import { Ionicons } from '@expo/vector-icons';
import { Montserrat_400Regular } from "@expo-google-fonts/montserrat";
import useServerIP from "./useServerIP";

const Header = () => {
    // Estado para guardar a porcentagem da bateria
    const [bateria, setBateria] = useState<number | null>(null);
    const ip = useServerIP(); // Utiliza sua lógica de IP automático

    useEffect(() => {
        const fetchBateria = async () => {
            try {
                // Adiciona um timestamp para evitar cache no GET
                const timestamp = new Date().getTime(); 
                const response = await fetch("http://" + ip + ":8080/animalInfo?t=" + timestamp);
                const data = await response.json();
                
                // Assume que o backend retorna os dados do JSON completo
                // e que o campo 'bateria' está dentro do primeiro pet retornado
                if(data.dados && data.dados.length > 0 && data.dados[0].bateria !== undefined) {
                    setBateria(data.dados[0].bateria);
                }
            } catch (error) {
                console.error("Erro ao buscar bateria:", error);
            }
        };

        fetchBateria(); // Chamada inicial

        // Atualiza o valor a cada 60 segundos
        const intervalo = setInterval(fetchBateria, 60000);
        return () => clearInterval(intervalo);
    }, [ip]);

    // Define o ícone baseado na porcentagem
    const getIconeBateria = (nivel: number | null) => {
        if (nivel === null) return "battery-dead";
        if (nivel >= 80) return "battery-full";
        if (nivel >= 40) return "battery-half";
        return "battery-dead";
    };

    return(
        <View style={{width:'100%', flexDirection:'row', alignItems:'center', justifyContent:"space-between", backgroundColor:'white'}}>
            <Image source={require("../assets/logo.png")} style={{width:150, height:100, marginLeft:10}}></Image>
            <View style={{flexDirection: 'row', alignItems:'center', marginRight:30}}>
                <Text style={{color: "grey", marginRight:6, fontFamily: 'Montserrat_400Regular'}}>
                    {/* Renderiza o valor com tratamento para o estado nulo */}
                    {bateria !== null ? `${bateria}%` : '--%'}
                </Text>
                <Ionicons name={getIconeBateria(bateria)} size={40} color="grey" />
            </View>
        </View>
    );
}

export default Header;