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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Trabajo");

// Requisitos
#define REQUISITO_TASA_LLAM "64kbps"
#define REQUISITO_RETARDO_MAX "140ms"
#define REQUISITO_PORCEN_LLAM_CORRECTAS 99.99
#define DEFAULT_NUM_CLIENTES 5
#define DEFAULT_TASA_CENTRALES "1Mbps"
//Como configuración por defecto del cliente se pretende que esté en condiciones normales.
#define DEFAULT_CLIENTES_CONEXION "1Mbps"
#define DEFAULT_CLIENTES_RETARDO "2ms"
#define DEFAULT_CLIENTES_PERROR_BIT 0.00001
#define DEFAULT_CLIENTES_DURACION_LLAMADA "20s"
#define DEFAULT_CLIENTES_SILENCIO "5min"

//retardo entre centrales
#define DEFAULT_CENTRALES_RETARDO "10ms"

#define NUM_GRAFICAS 3
#define GRAFICA_NODOS 0

#define IC_SIMULACIONES_POR_PUNTO 10
#define IC_PORCEN 95
#define IC_PONDERACION 2.2622

struct DATOS {
};

bool
cumpleRequisitos ();

void
simulacion (uint32_t nClientesPorCentral, Ptr<ExponentialRandomVariable> velEnlace, 
	    Ptr<ExponentialRandomVariable> retEnlace, Ptr<ExponentialRandomVariable> onTime, 
	    Ptr<ExponentialRandomVariable> offTime, DataRate dataRate, uint32_t sizePkt);

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Modo de simulacion
  bool calcularNodos = false;

  // Configuracion de requisitos y especificaciones
  DataRate tasaLlamMinima =DataRate( REQUISITO_TASA_LLAM);
  Time retardoMaximo = Time(REQUISITO_RETARDO_MAX);
  double porcenLlamadasCorrectasMin = REQUISITO_PORCEN_LLAM_CORRECTAS;

  // Configuracion de escenario
  uint32_t nClientesPorCentral = DEFAULT_NUM_CLIENTES; 
  
  // Configuracion de clientes
  DataRate conexClientesMedia =DataRate (DEFAULT_CLIENTES_CONEXION);
  Time retardoClientesMedia = Time(DEFAULT_CLIENTES_RETARDO);
  uint32_t probErrorBitClientesMedia = DEFAULT_CLIENTES_PERROR_BIT;
  Time duracionLlamClientesMedia = Time(DEFAULT_CLIENTES_DURACION_LLAMADA);
  Time duracionSilencioClientesMedia =Time(DEFAULT_CLIENTES_SILENCIO);
  DataRate velocidad = DataRate ("1Mbps");
  Ptr<ExponentialRandomVariable> velEnlace = CreateObject<ExponentialRandomVariable> ();
  velEnlace -> SetAttribute("Mean", DoubleValue (double(velocidad.GetBitRate())));
  Ptr<ExponentialRandomVariable> retEnlace = CreateObject<ExponentialRandomVariable> ();
  retEnlace -> SetAttribute("Mean", DoubleValue (0.002));//para meterle 2ms

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("calculoNodos", "Calculo de numero de nodos optimo por central", calcularNodos);
  cmd.AddValue("conexClientesMedia", "conexion media de clientes", conexClientesMedia);
  cmd.AddValue("retardoClientesMedia", "retardo medio en los clientes", retardoClientesMedia);
  cmd.AddValue("probErrorBitClientesMedia", "probabilidad de error de bit media en clientes", probErrorBitClientesMedia);
  cmd.AddValue("duracionLlamClientesMedia", "duración media de llamadas en los clientes", duracionLlamClientesMedia);
  cmd.AddValue("duracionSilencioClientesMedia", "duacion media de silencios en los clientes", duracionSilencioClientesMedia);
  cmd.AddValue("nClientesPorCentral","numero de nodos por central",nClientesPorCentral);
  cmd.Parse (argc, argv);

  /*
  // Graficas:
  // - Calculo de numero de nodos:
  //   * Porcent llam OK vs numero de nodos
  //   * Retardo minimo vs numero de nodos
  //   * Tasa minima vs numero de nodos
  // - Calculo de parametros:
  //   * TODO
	Gnuplot graficas[NUM_GRAFICAS];
  Gnuplot2dDataset curvas[NUM_GRAFICAS];
	std::ostringstream tituloGraficas[NUM_GRAFICAS];

	if (calcularNodos) {
		// Modo de calculo de nodos: Obtener el numero de nodos maximo
		// Configurar parametros de entrada de la grafica
		tituloGraficas[GRAFICA_NODOS]
			<< "Numero de nodos total en centrales (distribuidos equitativamente)"
			<< "(intervalo confianza: " << IC_PORCEN << "%)\\n"
			<< parametrosEntrada.str ();
		graficas[GRAFICA_NODOS].SetTitle (tituloGraficas[GRAFICA_NODOS]);
		graficas[GRAFICA_NODOS].SetLegend (
			// Eje X
			"Numero de nodos total en las 2 centrales (distribuidos equitativamente)"
			// Eje Y
			"Porcentaje de mensajes de voz validos (%)"
		);
		// Configurar el algoritmo de calculo de numero de clientes
		CalculoClientes instanciaCalculoClientes (
			DEFAULT_TASA_CENTRALES, REQUISITO_TASA_LLAM
		);
		uint32_t maxNumClientes = instanciaCalculoClientes.GetInitialValue ();
		// Ejecucion del algoritmo de calculo de clientes
		while (! instanciaCalculoClientes.FoundValue ()) {
			NS_LOG_DEBUG ("Iteracion: " << maxNumClientes << " clientes");
      // TODO Obtener punto e IC segun numero de nodos analizado
      Average<double> porcentCumplimLlam;
      double IC[NUM_GRAFICAS];
      for (int simul = 0; simul < IC_SIMULACIONES_POR_PUNTO; simul++) {
        DATOS resultadoSimulacion = simulacion (maxNumClientes, ...);
        porcentCumplimLlam.Update (resultadoSimulacion.porcentCumplimientoLlam);
        // TODO: Grafica retardos?
        // TODO: Grafica tasa?
      }
      IC[GRAFICA_NODOS] = IC_PONDERACION * sqrt (porcentCumplimLlam.Var () / IC_SIMULACIONES_POR_PUNTO);
      // Aniadir punto a la grafica
      curvas[GRAFICA_NODOS].Add (porcentCumplimLlam.Mean (), maxNumClientes, IC[GRAFICA_NODOS]);
      // El algoritmo se encarga de encontrar un valor optimo de nClientesPorCentral
			if (cumpleRequisitos (resultadoSimulacion)) {
				NS_LOG_DEBUG ("Cumplimiento de requisitos con " << maxNumClientes << " clientes");
				maxNumClientes = instanciaCalculoClientes.GetValue ();
			} else {
				NS_LOG_DEBUG ("No se cumplen los requisitos con " << maxNumClientes << " clientes");
				// Incumple nodos, volver al valor anterior
				maxNumClientes = instanciaCalculoClientes.ResetValue ();
			}
		}
    graficas[GRAFICA_NODOS].AddDataset (curvas [GRAFICA_NODOS]);
	}
  */

  // Siempre finaliza correctamente
  return 0;
}

bool
cumpleRequisitos ()
{
  // TODO
  return false;
}

void
simulacion (uint32_t nClientesPorCentral, Ptr<ExponentialRandomVariable> velEnlace, 
	    Ptr<ExponentialRandomVariable> retEnlace, Ptr<ExponentialRandomVariable> onTime, 
	    Ptr<ExponentialRandomVariable> offTime, DataRate dataRate, uint32_t sizePkt) {

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
  DeviceCentrales= pointToPointCentrales.Install (centrales);
  
  PointToPointHelper pointToPointNodos;
  NetDeviceContainer DeviceCentral1[nClientesPorCentral];
  NetDeviceContainer DeviceCentral2[nClientesPorCentral];
  //Agregacion de los atributos a los enlaces entre nodos y centrales
  for(uint32_t device=0; device<nClientesPorCentral;device++)
  {
    pointToPointNodos.SetDeviceAttribute ("DataRate",DataRateValue(DataRate(uint64_t(velEnlace->GetValue()))));
    pointToPointNodos.SetChannelAttribute ("Delay", TimeValue(Time(retEnlace->GetValue())));
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

  std::ostringstream direccionIP1; 
  std::ostringstream direccionIP2;
  uint32_t ip1=0;
  uint32_t ip2=0;
  Ipv4InterfaceContainer Interfacesnodos1[nClientesPorCentral];
  Ipv4InterfaceContainer Interfacesnodos2[nClientesPorCentral];

  for(uint32_t nclientes=1; nclientes <= nClientesPorCentral; nclientes++)
    {      
      if(ip1<255)
	{
	  direccionIP1 << "10."<< ip2 << "." << ip1 << ".0" ;
	  ip1++;
	  direccionIP2 << "10."<< ip2 << "." << ip1 << ".0" ;
	  ip1++;
	}
      else
	{
	  ip1=0;
	  ip2++;
	  direccionIP1 << "10."<< ip2 << "." << ip1 << ".0" ;	  
	  ip1++;
	  direccionIP1 << "10."<< ip2 << "." << ip1 << ".0" ;
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
    
   for (uint32_t nclientes=0; nclientes<nClientesPorCentral; nclientes++)
     {
       //Instalacion de sumideros
        ApplicationContainer app1 = sink1.Install (central1[nclientes].Get (0));
	ApplicationContainer app2 = sink2.Install (central2[nclientes].Get (0));

	//Creacion de clientes
       OnOffHelper clientes1 ("ns3::UdpSocketFactory",
			      Address (InetSocketAddress ( Interfacesnodos2[nclientes].GetAddress (0), port)));
       //Valores de los clientes concectados a la central1
       clientes1.SetAttribute("OnTime",PointerValue(onTime));
       clientes1.SetAttribute("OffTime",PointerValue(offTime));
       clientes1.SetAttribute("PacketSize",UintegerValue (sizePkt));
       clientes1.SetAttribute("DataRate",DataRateValue(dataRate));
       //para añadir el on/off a todos los nodos de la central1.
       ApplicationContainer clientApps1 = clientes1.Install (central1[nclientes]);
       OnOffHelper clientes2 ("ns3::UdpSocketFactory",
			      Address (InetSocketAddress ( Interfacesnodos1[nclientes].GetAddress (0), port)));
       //Valores de los clientes concectados a la central2
       clientes2.SetAttribute("OnTime",PointerValue(onTime));
       clientes2.SetAttribute("OffTime",PointerValue(offTime));
       clientes2.SetAttribute("PacketSize",UintegerValue (sizePkt));
       clientes2.SetAttribute("DataRate",DataRateValue(dataRate));
       //para añadir el on/off a todos los nodos de la central2.
       ApplicationContainer clientApps2 = clientes2.Install (central2[nclientes]);
     }
 
  /*
   //Valores de los clientes concectados a la central1
   clientes1.SetAttribute("OnTime",PointerValue(onTime));
   clientes1.SetAttribute("OffTime",PointerValue(offTime));
   clientes1.SetAttribute("PacketSize",UintegerValue (sizePkt));
   clientes1.SetAttribute("DataRate",DataRateValue(dataRate)); 
   //Valores de los clientes concectados a la central2
   clientes2.SetAttribute("OnTime",PointerValue(onTime));
   clientes2.SetAttribute("OffTime",PointerValue(offTime));
   clientes2.SetAttribute("PacketSize",UintegerValue (sizePkt));
   clientes2.SetAttribute("DataRate",DataRateValue(dataRate));
   for(uint32_t nclientes=0; nclientes< nClientesPorCentral; nclientes++)
     {
       //para añadir el on/off a todos los nodos de la central1.
       ApplicationContainer clientApps1 = clientes1.Install (central1[nclientes]);
       //para añadir el on/off a todos los nodos de la central2.
       ApplicationContainer clientApps2 = clientes2.Install (central2[nclientes]);
     }
   */

   clientApps1.Start (Seconds (2.0));
   clientApps2.Start (Seconds (2.0));
   clientApps1.Stop (Seconds (10.0));
   clientApps2.Stop (Seconds (10.0));
   
}

