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
#include "DireccionamientoIpv4Helper.h"
#include "LlamadasHelper.h"
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
#define DEFAULT_CENTRALES_PERROR_BIT 0.000005 // Prob. error de bit entre centrales
#define DEFAULT_CENTRALES_TAMCOLA 2

// Valores por defecto del los clientes
// Por defecto se supondra que esta en condiciones normales
#define DEFAULT_NUM_CLIENTES 100
#define DEFAULT_CLIENTES_TASA "1Mbps"
#define DEFAULT_CLIENTES_RETARDO "2ms"
#define DEFAULT_CLIENTES_DURACION_LLAMADA "5s"
#define DEFAULT_CLIENTES_PERROR_BIT 0.00005
// Probabilidad de que un cliente realice una llamada durante la simulacion
#define DEFAULT_CLIENTES_PROB_LLAMADA 0.1

// Configuracion del escenario
#define NUM_CENTRALES 2
#define APP_PORT 5000

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

// Configuracion de simulacion (en segundos)
#define START_TIME "1s"
#define STOP_TIME "5s"

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
  Ptr<ExponentialRandomVariable> durLlamVar, Ptr<UniformRandomVariable> tLlamVar,
  Ptr<UniformRandomVariable> probLlamVar, double pLlam,
  double pError, DataRate tasaLlam, uint32_t sizePkt, uint32_t tamCola
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
  double clientesProbLlam = DEFAULT_CLIENTES_PROB_LLAMADA;
  Time clientesDuracionMediaLlam (DEFAULT_CLIENTES_DURACION_LLAMADA);
  DataRate clientesCapacidadEnlaceMedia (DEFAULT_CLIENTES_TASA);
  Time clientesRetardoMedio (DEFAULT_CLIENTES_RETARDO);

  // Otras configuraciones
  // Tasa de transmision del protocolo usado
  DataRate protocoloTasa (REQUISITO_LLAM_TASA);
  // Tamaño del paquete (por simplicidad se considerara constante)
  uint32_t tamPaquete = DEFAULT_TAM_PAQUETE;

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("calculoClientes", "Modo de calculo de num de clientes optimo por central",
               modoCalculoClientes);
  cmd.AddValue("nClientesPorCentral", "Numero de clientes por central",
               nClientesPorCentral);
  cmd.AddValue("conex", "Velocidad de conexion media de clientes",
               clientesCapacidadEnlaceMedia);
  cmd.AddValue("delay", "Retardo medio en los clientes",
               clientesRetardoMedio);
  cmd.AddValue("durLlam", "Duracion media de llamada entre clientes",
               clientesDuracionMediaLlam);
  cmd.AddValue("pLlam", "Probabilidad de que un cliente realice una llamada en simulacion",
               clientesProbLlam);
  cmd.AddValue("pError", "Prob. error bit de enlaces de clientes (constante)",
               clientesProbErrorBit);
  cmd.AddValue("tasaVoz", "Tasa del protocolo de llamadas",
               protocoloTasa);
  cmd.AddValue("tamPkt", "Tamanio de paquetes transmitidos por clientes",
               tamPaquete);
  cmd.Parse (argc, argv);

  // Tratar los valores obtenidos/modificados por linea de comandos
  // Comprueba que el numero de clientes no sea 0, si lo fuera lo pone a 1.
  nClientesPorCentral = nClientesPorCentral <= 0 ? 1 : nClientesPorCentral;

  // Construir cadena de parametros de entrada usada en logs y graficas
  std::ostringstream parametrosEntrada;
  parametrosEntrada
    << "conex=" << clientesCapacidadEnlaceMedia.GetBitRate () / 1000000.0 << "Mbps "
    << "delay=" << clientesRetardoMedio.GetMicroSeconds () / 1000.0 << "ms "
    << "durLlam=" << clientesDuracionMediaLlam.GetMilliSeconds () / 1000.0 << "s "
    << "pLlam=" << clientesProbLlam << " "
    << "pError=" << clientesProbErrorBit << " "
    << "tasaVoz=" << protocoloTasa.GetBitRate () / 1000.0 << "kbps "
    << "tamPkt=" << tamPaquete << "B";
  NS_LOG_INFO (parametrosEntrada.str ());

  // Variables aleatorias para obtener valores unicos por clientes:
  // Capacidad del enlace de cada cliente (en bps)
  Ptr<ExponentialRandomVariable> clientesCapacidadEnlace = CreateObject<ExponentialRandomVariable> ();
  clientesCapacidadEnlace->SetAttribute("Mean", DoubleValue (clientesCapacidadEnlaceMedia.GetBitRate ()));
  // Retardo del enlace de cada cliente (en microsegundos)
  Ptr<ExponentialRandomVariable> clientesRetardo = CreateObject<ExponentialRandomVariable> ();
  clientesRetardo->SetAttribute("Mean", DoubleValue (clientesRetardoMedio.GetMicroSeconds () / 1000.0));
  // Duracion de cada llamada (en segundos)
  Ptr<ExponentialRandomVariable> durLlamVar = CreateObject<ExponentialRandomVariable> ();
  durLlamVar->SetAttribute("Mean", DoubleValue (
    Time (DEFAULT_CLIENTES_DURACION_LLAMADA).GetMilliSeconds () / 1000.0));
  // Tiempo de inicio de cada llamada (en segundos)
  Ptr<UniformRandomVariable> tLlamVar = CreateObject<UniformRandomVariable> ();
  tLlamVar->SetAttribute("Min", DoubleValue (
    Time (START_TIME).GetMilliSeconds () / 1000.0));
  tLlamVar->SetAttribute("Max", DoubleValue (
    Time (STOP_TIME).GetMilliSeconds () / 1000.0));
  // Probabilidad de que se produzca una llamada (en tanto por uno)
  Ptr<UniformRandomVariable> probLlamVar = CreateObject<UniformRandomVariable> ();
  // Usado para comparar con probabilidad de llamada (parametro)
  probLlamVar->SetAttribute ("Min", DoubleValue (0));
  probLlamVar->SetAttribute ("Max", DoubleValue (1));

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
            durLlamVar, tLlamVar, probLlamVar, clientesProbLlam,
            clientesProbErrorBit, protocoloTasa, tamPaquete, tamCola
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
    << "* Numero de simulaciones: " << contadorSimulaciones << "\n"
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
  Ptr<ExponentialRandomVariable> durLlamVar, Ptr<UniformRandomVariable> tLlamVar,
  Ptr<UniformRandomVariable> probLlamVar, double pLlam,
  double pError, DataRate tasaLlam, uint32_t sizePkt, uint32_t tamCola
) {
  NS_LOG_FUNCTION (numClientes << capacEnlace << delayEnlace << durLlamVar << tLlamVar
                   << probLlamVar << pLlam << pError << tasaLlam << sizePkt << tamCola);

  // -------------------------------- CENTRALES --------------------------------
  NS_LOG_DEBUG ("Creando centrales");
  // Nodos de centrales
  NodeContainer centrales;
  centrales.Create (NUM_CENTRALES);
  // Definiciones: Enlaces y dispositivos de centrales
  PointToPointHelper enlacesCentrales;
  NetDeviceContainer dispositivosCentrales;
  // Configurar modelo de probabilidad de error de bits entre centrales
  Ptr<RateErrorModel> pErrorCentrales = CreateObject<RateErrorModel> ();
  pErrorCentrales->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
  pErrorCentrales->SetRate (DEFAULT_CENTRALES_PERROR_BIT);
  // Configurar los enlaces entre las centrales
  enlacesCentrales.SetDeviceAttribute ("DataRate",
                                       StringValue (DEFAULT_CENTRALES_TASA));
  enlacesCentrales.SetChannelAttribute ("Delay",
                                        StringValue (DEFAULT_CENTRALES_RETARDO));
  enlacesCentrales.SetDeviceAttribute ("ReceiveErrorModel",
                                       PointerValue (pErrorCentrales));
  // El tamanio de cola sera variable
  enlacesCentrales.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (tamCola));
  // Crear el enlace entre las centrales
  dispositivosCentrales = enlacesCentrales.Install (centrales);

  // -------------------------------- CLIENTES ---------------------------------
  NS_LOG_DEBUG ("Creando clientes");
  NodeContainer clientes[NUM_CENTRALES];
  // Configurar modelo de probabilidad de error de bits entre centrales
  Ptr<RateErrorModel> pErrorClienteCentral = CreateObject<RateErrorModel> ();
  pErrorClienteCentral->SetUnit (RateErrorModel::ERROR_UNIT_BIT);
  pErrorClienteCentral->SetRate (DEFAULT_CLIENTES_PERROR_BIT);
  // Configurar enlaces y dispositivos con la central (punto a punto)
  // Notese que en total tendremos 2 * "numClientes" clientes
  // Cada central tendra "numClientes" clientes (parametro de simulacion ())
  NodeContainer* paresClienteCentral = new NodeContainer[2 * numClientes];
  PointToPointHelper* enlacesClienteCentral = new PointToPointHelper[2 * numClientes];
  NetDeviceContainer* dispClienteCentral = new NetDeviceContainer[2 * numClientes];
  // La logica de creacion de clientes es la misma en las dos centrales
  // Recorrer centrales y asociar nuevos nodos
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    clientes[idCentral].Create (numClientes + 1);
    // Asignar los pares cliente-central
    // De 0 a n-1 para la central 1, de n a 2n-1 para la central 2
    for (uint32_t iteradorClientes = 0;
         iteradorClientes < numClientes;
         iteradorClientes++) {
      // Lograr una iteracion desde 0 hasta 2n - 1
      uint32_t idCliente = iteradorClientes + idCentral * numClientes;
      // Aniadir central al par cliente-central
      paresClienteCentral[idCliente].Add (centrales.Get (idCentral));
      // Lo mismo para clientes (un cliente distinto por par central cliente)
      paresClienteCentral[idCliente].Add (clientes[idCentral].Get (iteradorClientes));
      // Configurar los parametros del enlace: Tasa, retardo y errores
      enlacesClienteCentral[idCliente].SetDeviceAttribute ("DataRate",
        DataRateValue (DataRate (capacEnlace->GetValue ())));
      enlacesClienteCentral[idCliente].SetChannelAttribute ("Delay",
        TimeValue (MilliSeconds (delayEnlace->GetValue ())));
      enlacesClienteCentral[idCliente].SetDeviceAttribute ("ReceiveErrorModel",
        PointerValue (pErrorClienteCentral));
      // Instalar los dispositivos en los nodos
      dispClienteCentral[idCliente] =
        enlacesClienteCentral[idCliente].Install (paresClienteCentral[idCliente]);
    }
  }
  // Fin recorrido de centrales
  // Asignamiento de llamadas (dos clientes comunicados entre si)
  // Los valores "duracionLlamadas" y "probLlamada" son variables aleatorias
  LlamadasHelper llamadas (numClientes, durLlamVar, tLlamVar, probLlamVar,
                           pLlam);
  // Imprimir la asignacion de llamadas
  NS_LOG_DEBUG (llamadas.GetAsignacion ());
  // ------------------------- CONFIGURACIONES DE RED --------------------------
  NS_LOG_DEBUG ("Instalando pila TCP/IP");
  // Instalamos la pila TCP/IP en todos los clientes y centrales
  InternetStackHelper pilaTcpIp;
  pilaTcpIp.Install (centrales);
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    pilaTcpIp.Install (clientes[idCentral]);
  }
  // Asignamiento de IPs, delegada a una clase
  DireccionamientoIpv4Helper direcciones;
  // Incluye subred para centrales
  Ipv4InterfaceContainer* subredes = new Ipv4InterfaceContainer[2 * numClientes + 1];
  // Red entre centrales (que es un poco especifica)
  subredes[0] = direcciones.CreateSubnet (numClientes * 2, dispositivosCentrales);
  for (uint32_t idCliente = 0; idCliente < 2 * numClientes; idCliente++) {
    // Notese que la subred #0 es la que conecta las centrales
    subredes[idCliente + 1] = direcciones.CreateSubnet (idCliente,
                                                        dispClienteCentral[idCliente]);
  }
  // Popular las tablas de enrutamiento, con el fin de que todos los clientes
  // de una central puedan acceder a los que pertenecen a esta, y tambien a los
  // clientes de la segunda central
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // Imprimir la estructura de la red
  NS_LOG_DEBUG (direcciones.ToString ());
  // Imprimir tablas de reenvio
  NS_LOG_DEBUG (direcciones.RoutingTables ());

  // ------------------------- APLICACIONES: SUMIDERO --------------------------
  NS_LOG_DEBUG ("Instalando sumideros en clientes");
  // Cada cliente (que no central) tendran instalados un sumidero
  // Esto servira para puedan recibir los paquetes
  // Establecemos el sumidero en un puerto (APP_PORT)
  PacketSinkHelper sumidero ("ns3::UdpSocketFactory",
                             Address (InetSocketAddress (Ipv4Address::GetAny (),
                                                         APP_PORT)));
  ApplicationContainer* appsSumidero = new ApplicationContainer[NUM_CENTRALES];
  // Instalamos el sumidero sobre todos los clientes disponibles
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    appsSumidero[idCentral] = sumidero.Install (clientes[idCentral]);
  }

  // ------------------------- APLICACIONES: LLAMADAS --------------------------
  NS_LOG_DEBUG ("Instalando aplicaciones de llamadas en clientes");
  // Inicializar las variables aleatorias a usar
  Ptr<ConstantRandomVariable> toff = CreateObject<ConstantRandomVariable> ();
  Ptr<ConstantRandomVariable> ton = CreateObject<ConstantRandomVariable> ();
  // Queremos que siempre transmita datos durante el funcionamiento de la aplicacion
  // Es decir, que "toff" siempre valga cero
  toff->SetAttribute ("Constant", DoubleValue (0));
  // El maximo valor posible para la transmision es, obviamente, STOP_TIME
  ton->SetAttribute ("Constant", DoubleValue (Time (STOP_TIME).GetSeconds ()));
  // Cada cliente (que no central) tendran instalado un cliente de llamadas, que
  // modelaran un sistema de llamadas en el que se envia un flujo de trafico
  // constante (que representara la voz en el destino respectivo)
  ApplicationContainer* appsLlam = new ApplicationContainer[2 * numClientes];
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    // Identificador del cliente respecto a todas las centrales
    uint32_t idClienteTotal;
    for (uint32_t idCliente = 0; idCliente < numClientes; idCliente++) {
      idClienteTotal = idCentral * numClientes + idCliente;
      // Crear la aplicacion, con el destino escogido en "LlamadasHelper"
      OnOffHelper clienteLlam (
        "ns3::UdpSocketFactory",
        Address (InetSocketAddress (
          direcciones.GetIp (llamadas.GetIdDestino (idClienteTotal)),
          APP_PORT)));
      // Configurar tasa de transmision y tamanio de paquete
      clienteLlam.SetAttribute ("DataRate", DataRateValue (tasaLlam));
      clienteLlam.SetAttribute ("PacketSize", UintegerValue (sizePkt));
      // Siempre transmitir datos
      clienteLlam.SetAttribute ("OnTime", PointerValue (ton));
      clienteLlam.SetAttribute ("OffTime", PointerValue (toff));
      // Instalar la aplicacion sobre un unico nodo
      // Notese que cada aplicacion tendra un destino distinto
      appsLlam[idClienteTotal] =
        clienteLlam.Install (clientes[idCentral].Get (idCliente));
    }
  }

  // ------------------------------- OBSERVADOR --------------------------------
  NS_LOG_DEBUG ("Configurando observador");
  // El observador se encargara de analizar las trazas de transmision y
  // recepcion de paquetes de toda la red, sin interferir en el funcionamiento
  // de la misma, con el fin de obtener los datos requeridos para realizar
  // las graficas del programa
  Observador observador;
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    // Identificador del cliente respecto a todas las centrales
    uint32_t idClienteTotal;
    for (uint32_t idCliente = 0; idCliente < numClientes; idCliente++) {
      idClienteTotal = idCentral * numClientes + idCliente;
      // Asociar las trazas de transmision de paquetes de cada cliente (no central)
      appsLlam[idClienteTotal].Get (0)
        ->GetObject<OnOffApplication> ()
        ->TraceConnectWithoutContext ("Tx", MakeCallback (&Observador::ActualizaTinicio,
                                                          &observador));
      // Establecer los tiempos de inicio y final de cada llamada
      appsLlam[idClienteTotal].Start (llamadas.GetStartTime (idClienteTotal));
      appsLlam[idClienteTotal].Stop (llamadas.GetStopTime (idClienteTotal));
      // Asociar las trazas de recepcion de todos los sumideros
      appsSumidero[idCentral].Get (idCliente)
        ->GetObject<PacketSink> ()
        ->TraceConnectWithoutContext ("Rx", MakeCallback (&Observador::ActualizaRetardos,
                                                          &observador));
    }
  }

  // ------------------- SIMULACION Y RECOPILACION DE DATOS --------------------
  NS_LOG_DEBUG ("Ejecutando simulacion");
  // La simulacion debe parar en STOP_TIME
  // Simulator::Stop (Time (STOP_TIME));
  // Lanzamos la simulacion
  Simulator::Run ();
  Simulator::Destroy ();
  // Insertar resultados en la estructura deseada
  RESULTADOS_SIMULACION resultados = {
    // Porcentaje de llamadas validas
    observador.GetMediaCorrectos (),
    // Retardo medio de llamada
    observador.GetMediaRetardos ()
  };
  return resultados;

  /* LOGICA ANTERIOR */
  /*
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
}

