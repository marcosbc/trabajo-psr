/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include <ns3/core-module.h>
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/gnuplot.h"

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


void
simulacion ();

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Configuracion de requisitos y especificaciones
  DataRate tasaLlamMinima = REQUISITO_TASA_LLAM;
  Time retardoMaximo = REQUISITO_RETARDO_MAX;
  double porcenLlamadasCorrectasMin = REQUISITO_PORCEN_LLAM_CORRECTAS;

  // Configuracion de escenario
  uint32_t nClientesPorCentral = DEFAULT_NUM_CLIENTES;
 
  
  // Configuracion de clientes
  DataRate conexClientesMedia = DEFAULT_CLIENTES_CONEXION;
  Time retardoClientesMedia = DEFAULT_CLIENTES_RETARDO;
  uint32_t probErrorBitClientesMedia = DEFAULT_CLIENTES_PERROR_BIT;
  Time duracionLlamClientesMedia = DEFAULT_CLIENTES_DURACION_LLAMADA;
  Time duracionSilencioClientesMedia = DEFAULT_CLIENTES_SILENCIO;
  DataRate velocidad = DataRate ("1Mbps");
  Ptr<ExponentialRandomVariable> velEnlace = CreateObject<ExponentialRandomVariable> ();
  velEnlace -> SetAttribute("Mean", DoubleValue (double(velocidad.GetBitRate())));
  Ptr<ExponentialRandomVariable> retEnlace = CreateObject<ExponentialRandomVariable> ();
  retEnlace -> SetAttribute("Mean", DoubleValue (0.002));//para meterle 2ms
   
  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("conexClientesMedia", "conexion media de clientes", conexClientesMedia);
  cmd.AddValue("retardoClientesMedia", "retardo medio en los clientes", retardoClientesMedia);
  cmd.AddValue("probErrorBitClientesMedia", "probabilidad de error de bit media en clientes", probErrorBitClientesMedia);
  cmd.AddValue("duracionLlamClientesMedia", "duración media de llamadas en los clientes", duracionLlamClientesMedia);
  cmd.AddValue("duracionSilencioClientesMedia", "duacion media de silencios en los clientes", duracionSilencioClientesMedia);
  cmd.AddValue("nClientesPorCentral","numero de nodos por central",nClientesPorCentral);
  cmd.Parse (argc, argv);
  
  simulacion ();
  
  

  // Siempre finaliza correctamente
  return 0;
}

void
simulacion (uint32_t nClientesPorCentral, Ptr<ExponentialRandomVariable> velEnlace,  Ptr<ExponentialRandomVariable> retEnlace) {

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
    pointToPointNodos.SetDeviceAttribute ("DataRate",DataRateValue(DataRate(uint64_t(velEnlace.GetValue()))));
    pointToPointNodos.SetChannelAttribute ("Delay", TimeValue(Time(retEnlace.GetValue())));
    DeviceCentral1[device]= pointToPointNodos.Install (central1[device]);
    DeviceCentral2[device]= pointToPointNodos.Install (central2[device]);
  }

  //Instalamos las pilas en todos los nodos y centrales.
  InternetStackHelper stack;
  stack.Install (p2pNodes1);
  stack.Install (p2pNodes2);

  Ipv4AddressHelper address;
  address.SetBase ("10.254.0.0", "255.255.255.0");
  Ipv4InterfaceContainer Interfacescentrales;
  Interfacescentrales = address.Assign (DeviceCentrales);


  std::ostringstream rotulocurvagraficas; //creación del rotulo de las curvas de las graficas.
  
  for(uint32_t ip=1; ip < 255; ip++)
    {
      if(ip<254)
	{
	  rotulocurvagraficas << "10.1." << ip << ".0" ;
	}
      else
	{
	  
	}
      rotulocurvagraficas.str();
    }
  

}

