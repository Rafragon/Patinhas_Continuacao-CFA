import { calcularMediaPassos, estimarPassosDiarios } from '@/patinhas-components/Estimador';
import Header from '@/patinhas-components/Header';
import useServerIP from '@/patinhas-components/useServerIP';
import React, { useEffect, useState } from 'react';
import { Dimensions, View, StyleSheet, Text, ScrollView } from 'react-native';
import { StackedBarChart } from 'react-native-chart-kit';

const chartConfig = {
  backgroundGradientFrom: '#ffffff',
  backgroundGradientTo: '#ffffff',
  decimalPlaces: 0,

  color: (opacity = 1) => `rgba(0, 82, 204, ${opacity})`,
  labelColor: () => '#555',

  propsForBackgroundLines: {
    stroke: '#e6e6e6',
    strokeDasharray: "0",
  },

  propsForLabels: {
    fontSize: 13,
    fontWeight: '600',
    // fontFamily: 'Montserrat_400Regular'
  },

  barPercentage: 0.55,
};


export default function graficoPassos() {
  const [pets, setPets] = useState([]);
  const [loading, setLoading] = useState(true);
  const [nome, setNome] = useState();
  const [animal, setAnimal] = useState({ pet: {} });

  const ip = useServerIP();
  useEffect(() => {
    const fetchPets = async () => {
      try {
        const response = await fetch("http://" + ip + ":8080/animalInfo");
        const data = await response.json();
        setPets(data["dados"][0]["estados"]);        // salva a lista de estados
        setNome(data["dados"][0]["nome"]);
        setAnimal({ pet: data["dados"][0] });
      } catch (error) {
        console.error("Erro ao buscar pets:", error);
      } finally {
        setLoading(false);
      }
    };

    fetchPets();
  }, []); // roda só 1 vez

  const estadosRecentes = [...pets]
    .sort((a: any, b: any) => new Date(b.data).getTime() - new Date(a.data).getTime())
    .slice(0, 5)
    .reverse();

  const labels = estadosRecentes.map((item: any) => {
    const [ano, mes, dia] = item.data.split("-");
    return `${dia}/${mes}`;
  });

  const data = estadosRecentes.map((item: any) => { const total = item.correndo + item.andando + item.parado + item.se_cocando; if (total === 0) return [0, 0, 0, 0]; return [+Number((item.correndo / total * 100).toFixed(0)), +Number((item.andando / total * 100).toFixed(0)), +Number((item.parado / total * 100).toFixed(0)), +Number((item.se_cocando / total * 100).toFixed(0))]; });
  const legend = ["Correndo", "Andando", "Parado", "Coçando"];
  const barColors = ["#4CAF50", "#FFC107", "#F44336", "#9E9E9E"];

  const estimativa = estimarPassosDiarios(animal);
  const passosIdeal = estimativa.estimate;

  return (
    <ScrollView>
      <Header></Header>

      <View style={[styles.chartCard, { marginTop: 20, width: '100%' }]}>

        <View style={{ alignItems: "center", marginBottom: 12 }}>
          <Text style={styles.chartTitle}>Passos do {nome} nos últimos 5 dias</Text>
        </View>

        {data.length > 0 && (<StackedBarChart
          style={{ borderRadius: 15, margin: "auto" }}
          data={{ labels, legend, data, barColors }}
          width={Dimensions.get('window').width - 60}
          fromZero={true}
          withInnerLines={true}
          height={350}
          yLabelsOffset={0}
          formatYLabel={(value) => `${parseInt(value, 10)}%`}
          chartConfig={chartConfig}
          verticalLabelRotation={0}
        />)}


        {/* <View style={{ alignItems: "center", marginBottom: 12 }}>
          <Text style={{}}>Média: {calcularMediaPassos(animal, 5).toFixed(1)}</Text>
          <Text style={{}}>Ideal: {passosIdeal}</Text>
        </View> */}
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  chartCard: {
    backgroundColor: '#ffffff',
    borderRadius: 16,
    padding: 16,
    marginTop: 20,
    width: '92%',
    alignSelf: 'center',

    // sombra moderna
    shadowColor: '#000',
    shadowOpacity: 0.1,
    shadowRadius: 12,
    shadowOffset: { width: 0, height: 4 },
    elevation: 4,
  },

  chartTitle: {
    fontSize: 20,
    fontWeight: "600",
    color: "#333",
    marginTop: 10,
    marginBottom: 6,
  }
});

