/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#define TRABAJO

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/applications-module.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/random-variable-stream.h>
#include <ns3/gnuplot.h>
#include "CalculoClientes.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Trabajo");

// Definicion de requisitos
#define REQUISITO_LLAM_TASA "64kbps"
#define REQUISITO_LLAM_RETARDO_MAX "140ms"
#define REQUISITO_PORCEN_LLAM_CORRECTAS 99

// Valores por defecto del escenario
#define DEFAULT_CENTRALES_TASA "1Mbps" // Tasa de transmision entre centrales
#define DEFAULT_CENTRALES_RETARDO "10ms" // Retardo entre centrales
#define DEFAULT_CENTRALES_TAMCOLA 2

// Valores por defecto del los clientes
// Por defecto se supondra que esta en condiciones normales
#define DEFAULT_NUM_CLIENTES 200
#define DEFAULT_CLIENTES_TASA "1Mbps"
#define DEFAULT_CLIENTES_RETARDO "2ms"
#define DEFAULT_CLIENTES_DURACION_LLAMADA "5s"
#define DEFAULT_CLIENTES_SILENCIO "20s"
#define DEFAULT_CLIENTES_PERROR_BIT 0.00005

// Configuracion del escenario
#define NUM_CENTRALES 2

// Configuracion de paquetes
#define DEFAULT_TAM_PAQUETE 400

// Configuracion de graficas
#define NUM_GRAFICAS 2
#define GRAFICA_CUMPLIM 0
#define GRAFICA_RETARDO 1
// Constantes para el intervalo de confianza.
#define IC_SIMULACIONES_POR_PUNTO 8
#define IC_PORCENTAJE 95
#define IC_PONDERACION 2.2622

struct RESULTADOS_SIMULACION {
  // Tanto por cien de llamadas consideradas validas
  double porcenLlamValidas;
  // Retardo medio de paquetes, en una estructura Time
  Time retardoMedioLlam;
};

bool
cumpleRequisitos (double porcenLlamValidas);

RESULTADOS_SIMULACION
simulacion (
  uint32_t numClientes,
  Ptr<ExponentialRandomVariable> capacEnlace, Ptr<ExponentialRandomVariable> delayEnlace,
  Ptr<ExponentialRandomVariable> ton, Ptr<ExponentialRandomVariable> toff,
  double pError, DataRate tasaLlam, uint32_t sizePkt
);

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Contador de simulaciones, por motivos de debug
  int contadorSimulaciones = 0;

  // Numero de clientes maximo simulado, para representar la recta de requisitos
  uint32_t numClientesMax = 0;

  // Modo de simulacion
  // TODO: Cambiar a false
  bool modoCalculoClientes = true;

  // Configuracion de escenario
  uint32_t nClientesPorCentral = DEFAULT_NUM_CLIENTES;

  // Configuracion de clientes
  double clientesProbErrorBit = DEFAULT_CLIENTES_PERROR_BIT;
  Time clientesDuracionMediaTx (DEFAULT_CLIENTES_DURACION_LLAMADA);
  Time clientesDuracionMediaSilencio (DEFAULT_CLIENTES_SILENCIO);
  DataRate clientesCapacidadEnlaceMedia (DEFAULT_CLIENTES_TASA);
  Time clientesRetardoMedio (DEFAULT_CLIENTES_RETARDO);

  // Otras configuraciones
  // Tasa de transmision del protocolo usado
  DataRate protocoloTasa (REQUISITO_LLAM_TASA);
  // Tamaño del paquete (por simplicidad se considerara constante)
  uint32_t tamPaquete = DEFAULT_TAM_PAQUETE;

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("calculoClientes", "Modo de calculo de numero de clientes optimo por central", modoCalculoClientes);
  cmd.AddValue("nClientesPorCentral", "Numero de clientes por central", nClientesPorCentral);
  cmd.AddValue("conex", "Velocidad de conexion media de clientes", clientesCapacidadEnlaceMedia);
  cmd.AddValue("delay", "Retardo medio en los clientes", clientesRetardoMedio);
  cmd.AddValue("ton", "Duracion media de transmision de los clientes", clientesDuracionMediaTx);
  cmd.AddValue("toff", "Duracion media de tiempo de silencios los clientes tras una transmision", clientesDuracionMediaSilencio);
  cmd.AddValue("pError", "Probabilidad de error de bit de los enlaces de clientes (constante)", clientesProbErrorBit);
  cmd.AddValue("tasaVoz", "Tasa durante la transmision de clientes (tasa del protocolo de llamadas)", protocoloTasa);
  cmd.AddValue("tamPkt", "Tamanio de paquetes transmitidos por clientes", tamPaquete);
  cmd.Parse (argc, argv);

  // Tratar los valores obtenidos/modificados por linea de comandos
  // Comprueba que el numero de clientes no sea 0, si lo fuera lo pone a 1.
  nClientesPorCentral = nClientesPorCentral <= 0 ? 1 : nClientesPorCentral;

  // Construir cadena de parametros de entrada usada en logs y graficas
  std::ostringstream parametrosEntrada;
  parametrosEntrada
    << "conex=" << clientesCapacidadEnlaceMedia.GetBitRate () / 1000000.0 << "Mbps "
    << "delay=" << clientesRetardoMedio.GetMicroSeconds () / 1000.0 << "ms "
    << "ton=" << clientesDuracionMediaTx.GetMilliSeconds () / 1000.0 << "s "
    << "toff=" << clientesDuracionMediaSilencio.GetMilliSeconds () / 1000.0 << "s "
    << "pError=" << clientesProbErrorBit << " "
    << "tasaVoz=" << protocoloTasa.GetBitRate () / 1000.0 << "kbps "
    << "tamPkt=" << tamPaquete << "B";
  NS_LOG_INFO (parametrosEntrada.str ());

  // Variables aleatorias para obtener valores unicos por clientes:
  // Tasas, retardos, duraciones de transmision y duraciones de silencio
  Ptr<ExponentialRandomVariable> clientesCapacidadEnlace = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> clientesRetardo = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> clientesDuracionTx = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> clientesDuracionSilencio = CreateObject<ExponentialRandomVariable> ();
  // Configurar las variables aleatorias
  clientesCapacidadEnlace->SetAttribute("Mean", DoubleValue (clientesCapacidadEnlaceMedia.GetBitRate ()));
  clientesRetardo->SetAttribute("Mean", DoubleValue (clientesRetardoMedio.GetMicroSeconds () / 1000.0));
  clientesDuracionTx->SetAttribute("Mean", DoubleValue (clientesDuracionMediaTx.GetMilliSeconds () / 1000.0));
  clientesDuracionSilencio->SetAttribute("Mean", DoubleValue (clientesDuracionMediaSilencio.GetMilliSeconds () / 1000.0));

  // Configuracion graficas:
  // - Calculo de numero de clientes:
  //   * Porcent llam OK vs numero de clientes
  //   * Retardo medio vs numero de clientes
  //   * Protocolos???
  //   * Porcent paquetes tx correctamente???
  //   Para las situaciones de baja, normal y alta actividad
  Gnuplot graficas[NUM_GRAFICAS];
  Gnuplot2dDataset curvas[NUM_GRAFICAS][DEFAULT_CENTRALES_TAMCOLA];
  std::ostringstream tituloGraficas[NUM_GRAFICAS];
  // Graficas de calculo de numero de clientes
  // 1 - Grafica de % de cumplimiento de llamadas
  tituloGraficas[GRAFICA_CUMPLIM]
    << "Numero de clientes por central, "
    << REQUISITO_PORCEN_LLAM_CORRECTAS << "\% de llamadas validas "
    << "(intervalo confianza: " << IC_PORCENTAJE << "%)\\n"
    << parametrosEntrada.str ();
  graficas[GRAFICA_CUMPLIM].SetTitle (tituloGraficas[GRAFICA_CUMPLIM].str ());
  graficas[GRAFICA_CUMPLIM].SetLegend (
    // Eje X
    "Numero de clientes por central ",
    // Eje Y
    "Porcentaje de mensajes de voz validos (%)"
  );
  // 2 - Grafica de retardos medios
  tituloGraficas[GRAFICA_RETARDO]
    << "Numero de clientes por central, "
    << REQUISITO_PORCEN_LLAM_CORRECTAS << "\% de llamadas validas "
    << "(intervalo confianza: " << IC_PORCENTAJE << "%)\\n"
    << parametrosEntrada.str ();
  graficas[GRAFICA_RETARDO].SetTitle (tituloGraficas[GRAFICA_RETARDO].str ());
  graficas[GRAFICA_RETARDO].SetLegend (
    // Eje X
    "Numero de clientes por central ",
    // Eje Y
    "Retardo medio de paquete (ms)"
  );
  // Configurar las curvas de las graficas
  std::ostringstream leyendaCurvas[DEFAULT_CENTRALES_TAMCOLA];
  for (uint32_t tamCola = 1; tamCola <= DEFAULT_CENTRALES_TAMCOLA; tamCola++) {
    leyendaCurvas[tamCola - 1] << "tamCola: " << tamCola;
    for (int idGrafica = 0; idGrafica < NUM_GRAFICAS; idGrafica++) {
      curvas[idGrafica][tamCola - 1].SetTitle (leyendaCurvas[tamCola - 1].str ());
      curvas[idGrafica][tamCola - 1].SetStyle (Gnuplot2dDataset::LINES_POINTS);
      curvas[idGrafica][tamCola - 1].SetErrorBars (Gnuplot2dDataset::Y);
    }
  }

  // Modo de calculo de clientes
  // Obtiene un valor aproximado (redondeado a 10) del numero de clientes maximo
  // que soportaria una central, hasta que empiecen a producirse incumplimiento
  // del SLA de llamadas (definidas como requisitos anteriormente)
  if (modoCalculoClientes) {
    // Cada grafica tendra distintas curvas, una por tamanio de cola
    for (uint32_t tamCola = 1; tamCola <= DEFAULT_CENTRALES_TAMCOLA; tamCola++) {
      NS_LOG_DEBUG ("Iteracion tamanio de cola: " << tamCola);
      // Configurar el algoritmo de calculo de numero de clientes
      // Servira para obtener los valores de numero de clientes para la grafica
      CalculoClientes instanciaCalculoClientes (
        DEFAULT_CENTRALES_TASA, REQUISITO_LLAM_TASA
      );
      uint32_t numClientes = instanciaCalculoClientes.GetInitialValue ();
      numClientesMax = numClientes;
      // Cada iteracion representara un distinto punto en el eje X de la grafica
      while (! instanciaCalculoClientes.FoundValue ()) {
        NS_LOG_DEBUG ("Iteracion clientes: " << numClientes << " clientes");
        // Obtener punto e IC segun numero de clientes analizado
        Average<double> porcenLlamValidas;
        Average<double> retardoMedioLlam;
        double IC[NUM_GRAFICAS];
        for (int simul = 0; simul < IC_SIMULACIONES_POR_PUNTO; simul++) {
          NS_LOG_DEBUG ("Iteracion IC: " << simul);
          // Ejecutar las simulaciones y obtener los datos
          RESULTADOS_SIMULACION result = simulacion (
            numClientes, clientesCapacidadEnlace, clientesRetardo,
            clientesDuracionTx, clientesDuracionSilencio,
            clientesProbErrorBit, protocoloTasa, tamPaquete
          );
          contadorSimulaciones++;
          NS_LOG_DEBUG ("Resultado simulacion " << contadorSimulaciones << ": "
            << "porcenLlamValidas = " << result.porcenLlamValidas << "%, "
            << "retardoMedioLlam = " << result.retardoMedioLlam.GetMicroSeconds () / 1000.0 << "ms");
          porcenLlamValidas.Update (result.porcenLlamValidas);
          retardoMedioLlam.Update (result.retardoMedioLlam.GetMicroSeconds () / 1000.0);
        }
        // Calcular el intervalo de confianza
        IC[GRAFICA_CUMPLIM] = IC_PONDERACION * sqrt (porcenLlamValidas.Var () / IC_SIMULACIONES_POR_PUNTO);
        IC[GRAFICA_RETARDO] = IC_PONDERACION * sqrt (retardoMedioLlam.Var () / IC_SIMULACIONES_POR_PUNTO);
        // Aniadir punto a la curva
        curvas[GRAFICA_CUMPLIM][tamCola - 1].Add (
          numClientes, porcenLlamValidas.Mean (), IC[GRAFICA_CUMPLIM]
        );
        curvas[GRAFICA_RETARDO][tamCola - 1].Add (
          numClientes, retardoMedioLlam.Mean (), IC[GRAFICA_RETARDO]
        );
        // Crearemos una recta representando el requisito de llamadas validas
        if (numClientesMax < numClientes) {
          numClientesMax = numClientes;
        }
        // Esta parte se dedica a especificamente a ejecutar el algoritmo
        // El algoritmo se encarga de encontrar un valor optimo de nClientesPorCentral
        if (cumpleRequisitos (porcenLlamValidas.Mean ())) {
           NS_LOG_DEBUG ("Cumplimiento de requisitos con " << numClientes << " clientes");
           numClientes = instanciaCalculoClientes.GetValue ();
        } else {
           NS_LOG_DEBUG ("No se cumplen los requisitos con " << numClientes << " clientes");
           // Incumple clientes, volver al valor anterior
           numClientes = instanciaCalculoClientes.ResetValue ();
        }
      }
      NS_LOG_INFO ("Encontrado optimo numero de clientes (tamCola = " << tamCola << "): " << numClientes);
      // Fin ejecucion del algoritmo
      // Aniadir curvas a las graficas
      graficas[GRAFICA_CUMPLIM].AddDataset (curvas [GRAFICA_CUMPLIM][tamCola - 1]);
      graficas[GRAFICA_RETARDO].AddDataset (curvas [GRAFICA_RETARDO][tamCola - 1]);
    }
    // Fin de recorrido de las curvas
    // Representar el requisito en una linea horizontal
    // Aniadir requisito de llamadas validas en la grafica correspondiente
    std::ostringstream rectaRequisitoLlam;
    rectaRequisitoLlam
      << "set arrow from 0,"
      << (REQUISITO_PORCEN_LLAM_CORRECTAS) << " "
      << "to " << numClientesMax << ","
      << floor (REQUISITO_PORCEN_LLAM_CORRECTAS) << " "
      << "nohead";
    graficas[GRAFICA_CUMPLIM].AppendExtra (rectaRequisitoLlam.str ());
    // Arreglar el intervalo de las graficas creadas
    for (int idGrafica = 0; idGrafica < NUM_GRAFICAS; idGrafica++) {
      std::ostringstream intervaloNumClientes;
      intervaloNumClientes << "set xrange [0:+" << numClientesMax << "]";
      graficas[idGrafica].AppendExtra (intervaloNumClientes.str ());
    }
  }
  // Fin del modo de calculo de clientes

  // Crear las graficas e imprimirlas
  for (int idGrafica = 0; idGrafica < NUM_GRAFICAS; idGrafica++) {
    // Creacion del fichero
    std::ostringstream tituloFichero;
    tituloFichero << "trabajo8-" << idGrafica + 1 << ".plt";
    std::ofstream ficheroGrafica (std::string (tituloFichero.str ()).c_str ());
    // Imprimir la grafica en el fichero recien creado
    graficas[idGrafica].GenerateOutput (ficheroGrafica);
    ficheroGrafica << "pause -1" << std::endl;
    ficheroGrafica.close ();
  }

  // Informacion de debug sobre los resultados
  NS_LOG_DEBUG (
    "RESULTADOS GLOBALES DE LAS SIMULACIONES\n"
    << "* Numero de simulaciones: " << contadorSimulaciones
    << "* Numero de clientes maximo simulado: " << numClientesMax
  );

  // Siempre retornaremos 0
  return 0;
}

bool
cumpleRequisitos (double porcenLlamValidas)
{
  NS_LOG_FUNCTION (porcenLlamValidas);
  return porcenLlamValidas >= REQUISITO_PORCEN_LLAM_CORRECTAS;
}

RESULTADOS_SIMULACION
simulacion (
  uint32_t numClientes,
  Ptr<ExponentialRandomVariable> capacEnlace, Ptr<ExponentialRandomVariable> delayEnlace,
  Ptr<ExponentialRandomVariable> ton, Ptr<ExponentialRandomVariable> toff,
  double pError, DataRate tasaLlam, uint32_t sizePkt
) {
  RESULTADOS_SIMULACION resultados;
  resultados.porcenLlamValidas = 100.0 - numClientes * 0.01;
  resultados.retardoMedioLlam = Time ("2ms");

  /*
  NodeContainer clientes
  for (int i = 0; i < NUM_CENTRALES; i++) {
    NodeContainer centrales
  }



  NodeContainer p2pNodes1;
  p2pNodes1.Create (nClientesPorCentral+1);//Creamos todos los nodos junto con las centrales
  NodeContainer p2pNodes2;
  p2pNodes2.Create (nClientesPorCentral+1);//Creamos todos los nodos junto con las centrales

  NodeContainer centrales;
  centrales.Add(p2pNodes1.Get(0));
  centrales.Add(p2pNodes2.Get(0));

  NodeContainer central1[nClientesPorCentral];
  NodeContainer central2[nClientesPorCentral];

  //Asociacion de los nodos a las distintas centrales.
  for(uint32_t nodo=0; nodo<nClientesPorCentral; nodo++)
    {
      central1[nodo].Add(p2pNodes1.Get(0));
      central1[nodo].Add(p2pNodes1.Get(nodo+1));

      central2[nodo].Add(p2pNodes2.Get(0));
      central2[nodo].Add(p2pNodes2.Get(nodo+1));
    }


  PointToPointHelper pointToPointCentrales;
  pointToPointCentrales.SetDeviceAttribute ("DataRate", StringValue (DEFAULT_TASA_CENTRALES));
  pointToPointCentrales.SetChannelAttribute ("Delay", StringValue ( DEFAULT_CENTRALES_RETARDO));
  NetDeviceContainer DeviceCentrales;
  //Asignamos al canal los errores
  Ptr<RateErrorModel> errores = CreateObject<RateErrorModel> ();
  errores->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
  errores->SetRate (probErrorBitClientesMedia);
  pointToPointCentrales.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (errores));
  DeviceCentrales= pointToPointCentrales.Install (centrales);

  Ptr<PointToPointNetDevice> nodoenlace1= DeviceCentrales.Get(0)->GetObject<PointToPointNetDevice>();
  nodoenlace1->GetQueue()->GetObject<DropTailQueue>()->SetAttribute("MaxPackets",UintegerValue(tamcola));
  Ptr<PointToPointNetDevice> nodoenlace2= DeviceCentrales.Get(1)->GetObject<PointToPointNetDevice>();
  nodoenlace2->GetQueue()->GetObject<DropTailQueue>()->SetAttribute("MaxPackets",UintegerValue(tamcola));

  PointToPointHelper pointToPointNodos;
  NetDeviceContainer DeviceCentral1[nClientesPorCentral];
  NetDeviceContainer DeviceCentral2[nClientesPorCentral];


  //Agregacion de los atributos a los enlaces entre nodos y centrales
  for(uint32_t device=0; device<nClientesPorCentral;device++)
  {
    pointToPointNodos.SetDeviceAttribute ("DataRate",DataRateValue(DataRate(uint64_t(velEnlace->GetValue()))));
    pointToPointNodos.SetChannelAttribute ("Delay", TimeValue(Time(retEnlace->GetValue())));
    pointToPointNodos.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (errores));
    DeviceCentral1[device]= pointToPointNodos.Install (central1[device]);
    DeviceCentral2[device]= pointToPointNodos.Install (central2[device]);
  }

  //Instalamos las pilas en todos los nodos y centrales.
  InternetStackHelper stack;
  stack.Install (p2pNodes1);
  stack.Install (p2pNodes2);


  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  Ipv4AddressHelper address;
  address.SetBase ("10.254.0.0", "255.255.255.0");
  Ipv4InterfaceContainer Interfacescentrales;
  Interfacescentrales = address.Assign (DeviceCentrales);

  uint32_t ip1=1;
  uint32_t ip2=1;
  Ipv4InterfaceContainer Interfacesnodos1[nClientesPorCentral];
  Ipv4InterfaceContainer Interfacesnodos2[nClientesPorCentral];

  for(uint32_t nclientes=0; nclientes < nClientesPorCentral; nclientes++)
    {
      std::ostringstream direccionIP1;
      std::ostringstream direccionIP2;
      if(ip1<255)
      {
        direccionIP1 << "10."<< ip2 << "." << ip1 << ".0" ;
        ip1++;
        direccionIP2 << "10."<< ip2 << "." << ip1 << ".0" ;
        ip1++;
      }
      else
      {
        ip1=1;
        ip2++;
        direccionIP1 << "10."<< ip2 << "." << ip1 << ".0" ;
        ip1++;
        direccionIP2 << "10."<< ip2 << "." << ip1 << ".0" ;
        ip1++;
      }

      address.SetBase (direccionIP1.str().c_str(), "255.255.255.0");
      Interfacesnodos1[nclientes] = address.Assign ( DeviceCentral1[nclientes]);
      address.SetBase (direccionIP2.str().c_str(), "255.255.255.0");
      Interfacesnodos2[nclientes] = address.Assign ( DeviceCentral2[nclientes]);

    }

  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t port = 9;
    PacketSinkHelper sink1 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
    PacketSinkHelper sink2 ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

    ApplicationContainer clientApps1[nClientesPorCentral];
    ApplicationContainer clientApps2[nClientesPorCentral];
    ApplicationContainer app1[nClientesPorCentral];
    ApplicationContainer app2[nClientesPorCentral];

   for (uint32_t nclientes=0; nclientes<nClientesPorCentral; nclientes++)
     {
       //Instalacion de sumideros
       app1[nclientes] = sink1.Install (central1[nclientes].Get (0));
       app2[nclientes] = sink2.Install (central2[nclientes].Get (0));

       //Creacion de clientes
       OnOffHelper clientes1 ("ns3::UdpSocketFactory",
         Address (InetSocketAddress ( Interfacesnodos2[nclientes].GetAddress (0), port)));
       //Valores de los clientes concectados a la central1
       clientes1.SetAttribute("OnTime",PointerValue(onTime));
       clientes1.SetAttribute("OffTime",PointerValue(offTime));
       clientes1.SetAttribute("PacketSize",UintegerValue (sizePkt));
       clientes1.SetAttribute("DataRate",DataRateValue(tasaEnvioCliente));
       //para añadir el on/off a todos los nodos de la central1.
       clientApps1[nclientes] = clientes1.Install (central1[nclientes]);
       OnOffHelper clientes2 ("ns3::UdpSocketFactory",
         Address (InetSocketAddress ( Interfacesnodos1[nclientes].GetAddress (0), port)));
       //Valores de los clientes concectados a la central2
       clientes2.SetAttribute("OnTime",PointerValue(onTime));
       clientes2.SetAttribute("OffTime",PointerValue(offTime));
       clientes2.SetAttribute("PacketSize",UintegerValue (sizePkt));
       clientes2.SetAttribute("DataRate",DataRateValue(tasaEnvioCliente));
       //para añadir el on/off a todos los nodos de la central2.
       clientApps2[nclientes] = clientes2.Install (central2[nclientes]);
     }

    //Trazas
  Observador observador;

  for(uint32_t numclientes = 0; numclientes <nClientesPorCentral; numclientes++)
    {
      clientApps1[numclientes].Get(0)->TraceConnectWithoutContext ("Tx",MakeCallback(&Observador::ActualizaTinicio,&observador));
      clientApps2[numclientes].Get(0)->TraceConnectWithoutContext ("Tx",MakeCallback(&Observador::ActualizaTinicio,&observador));
      app1[numclientes].Get(0)->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::ActualizaRetardos,&observador));
      app2[numclientes].Get(0)->TraceConnectWithoutContext("Rx",MakeCallback(&Observador::ActualizaRetardos,&observador));
    }

  for(uint32_t nclientes=0; nclientes<nClientesPorCentral; nclientes++)
    {
      clientApps1[nclientes].Start (Seconds (2.0));
      clientApps2[nclientes].Start (Seconds (2.0));
      clientApps1[nclientes].Stop (Seconds (10.0));
      clientApps2[nclientes].Stop (Seconds (10.0));
    }

    // Lanzamos la simulación
  Simulator::Run ();
  Simulator::Destroy ();

   //Comprovamos que la estructura no este vacia y en caso de estarlo
  //saltara un warn.
  observador.CompruebaEstructura();

  //Guardamos la media de los retardos en "retardos".
  *(retardos)=observador.GetMediaRetardos();
  NS_LOG_INFO ("Media Retardos: " << observador.GetMediaRetardos() << "us");
  //Guardamos el porcentaje de paquetes correctos en "correctos"
  *(correctos)=observador.GetMediaCorrectos()*100;
  NS_LOG_INFO ("Porcentaje Correcto: " << observador.GetMediaCorrectos()*100 << "%");
  */

  return resultados;
}

