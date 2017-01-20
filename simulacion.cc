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
//#include "CalculoClientes.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Trabajo");

// Requisitos
#define REQUISITO_TASA_LLAM "64kbps"
#define REQUISITO_RETARDO_MAX "140ms"
#define REQUISITO_PORCEN_LLAM_CORRECTAS 99.99
#define DEFAULT_NUM_CLIENTES 500
#define DEFAULT_TASA_CENTRALES "1Mbps"
//Como configuración por defecto del cliente se pretende que esté en condiciones normales.
#define DEFAULT_CLIENTES_CONEXION "1Mbps"
#define DEFAULT_CLIENTES_RETARDO 0.002 //2ms
#define DEFAULT_CLIENTES_PERROR_BIT 0.00005
#define DEFAULT_CLIENTES_DURACION_LLAMADA 0.2 //en segundos
#define DEFAULT_CLIENTES_SILENCIO 0.8 //En segundos

//retardo entre centrales
#define DEFAULT_CENTRALES_RETARDO "10ms"


#define TAM_PAQUETE 400
#define TASA_ENVIO_CLIENTE "1Kbps"

#define  TAMCOLA 2

//Constantes para el intervalo de confianza.
#define NUM_IC 10
#define IC95 2.2622

#define NUM_NODOS_INICIAL 5    //1
#define NUM_NODOS_MAXIMOS 100 //500
#define NUM_NODOS_PASO (NUM_NODOS_MAXIMOS-NUM_NODOS_INICIAL)/15 //PAra representar 15 puntos.

#define TON_BAJA 0.3
#define TON_PASO 0.3
#define TON_MAXIMO 0.9

/*
struct DATOS {
};

bool
cumpleRequisitos ();
*/
void
simulacion (uint32_t nClientesPorCentral, Ptr<ExponentialRandomVariable> velEnlace, 
	    Ptr<ExponentialRandomVariable> retEnlace, Ptr<ExponentialRandomVariable> onTime, 
	    Ptr<ExponentialRandomVariable> offTime, DataRate tasaEnvioCliente, uint32_t sizePkt,
	    uint32_t tamcola, double probErrorBitClientesMedia,double* retardos, double* correctos);

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Modo de simulacion
  // bool calcularNodos = false;

  /*  // Configuracion de requisitos y especificaciones
  DataRate tasaLlamMinima =DataRate( REQUISITO_TASA_LLAM);
  Time retardoMaximo = Time(REQUISITO_RETARDO_MAX);
  double porcenLlamadasCorrectasMin = REQUISITO_PORCEN_LLAM_CORRECTAS;
  */
  // Configuracion de escenario
  uint32_t nClientesPorCentral = DEFAULT_NUM_CLIENTES; 
  
  // Configuracion de clientes
  double retardoClientesMedia = DEFAULT_CLIENTES_RETARDO;
  double probErrorBitClientesMedia = DEFAULT_CLIENTES_PERROR_BIT;
  double duracionLlamClientesMedia = DEFAULT_CLIENTES_DURACION_LLAMADA;
  double duracionSilencioClientesMedia =DEFAULT_CLIENTES_SILENCIO;
  DataRate velocidadEnlacen = DataRate (DEFAULT_CLIENTES_CONEXION);

  Ptr<ExponentialRandomVariable> onTime = CreateObject<ExponentialRandomVariable> ();
  Ptr<ExponentialRandomVariable> offTime = CreateObject<ExponentialRandomVariable> ();
 
  //Tamaño del paquete
  uint32_t sizePkt =  TAM_PAQUETE;
  //Variable que guarda la tasa de envio.
  DataRate tasaEnvioCliente = DataRate (TASA_ENVIO_CLIENTE);


  // Obtener parametros por linea de comandos
  CommandLine cmd;
  // cmd.AddValue("calculoNodos", "Calculo de numero de nodos optimo por central", calcularNodos);
  cmd.AddValue("velocidadEnlacen", "velocidad de conexion media de clientes", velocidadEnlacen);
  cmd.AddValue("retardoClientesMedia", "retardo medio en los clientes", retardoClientesMedia);
  cmd.AddValue("probErrorBitClientesMedia", "probabilidad de error de bit media en clientes", probErrorBitClientesMedia);
  cmd.AddValue("duracionLlamClientesMedia", "duración media de llamadas en los clientes", duracionLlamClientesMedia);
  cmd.AddValue("duracionSilencioClientesMedia", "duacion media de silencios en los clientes", duracionSilencioClientesMedia);
  cmd.AddValue("nClientesPorCentral","numero de nodos por central",nClientesPorCentral);
  cmd.Parse (argc, argv);


  Ptr<ExponentialRandomVariable> velEnlace = CreateObject<ExponentialRandomVariable> ();
  velEnlace -> SetAttribute("Mean", DoubleValue (double(velocidadEnlacen.GetBitRate())));
  Ptr<ExponentialRandomVariable> retEnlace = CreateObject<ExponentialRandomVariable> ();
  retEnlace -> SetAttribute("Mean", DoubleValue (retardoClientesMedia));//para meterle 2ms


   // Comprueba que el numero de nodos no sea 0, si lo fuera lo pone a 1.
  nClientesPorCentral = nClientesPorCentral == 0 ? 1 : nClientesPorCentral;

  //Creación de las graficas requeridas
  Gnuplot grafica1[3]; //creamos la grafica1.
  Gnuplot grafica2[3]; //creamos la grafica2.
  //Variables para poder realizar el Intervalo de confianza
  //Creamos 2 variables tipo average para guardar los puntos cada grafica.
  Average<double> puntos1[3]; 
  Average<double> puntos2[3];
  double media1[3]; //Guardara la media.
  double varianza1[3]; //Contendra el valor de la Varianza.
  double IC1[3]; //Intervalo de confianza grafica1.

  double media2[3]; //Guardara la media .
  double varianza2[3]; //Contendra el valor de la Varianza.
  double IC2[3]; //Intervalo de confianza grafica2.
  
  // Para los rotulos de las graficas.
  uint32_t contador=0;

  /*Bucle para crear puntos variando el tamaño de la cola.*/
 for(uint32_t tamcola=1; tamcola <= TAMCOLA; tamcola++)
    { 
      NS_LOG_INFO ("Tamaño cola: " << tamcola);

      Gnuplot2dDataset curvagrafica1[3]; //Creación de las curvas de la grafica1.
      Gnuplot2dDataset curvagrafica2[3]; //Creación de las curvas de la grafica2
      std::ostringstream rotulocurvagraficas; //creación del rotulo de las curvas de las graficas.

      rotulocurvagraficas << "TamCola: " << contador+1 ;
      for(uint32_t contador1=0; contador1 < 3;contador1++)
	{
	  curvagrafica1[contador1].SetTitle(rotulocurvagraficas.str());
	  curvagrafica2[contador1].SetTitle(rotulocurvagraficas.str());
	  curvagrafica1[contador1].SetErrorBars (Gnuplot2dDataset::Y); 
	  curvagrafica2[contador1].SetErrorBars (Gnuplot2dDataset::Y);
	  curvagrafica1[contador1].SetStyle(Gnuplot2dDataset::LINES_POINTS);
	  curvagrafica2[contador1].SetStyle(Gnuplot2dDataset::LINES_POINTS);
	}
      //Bucle para las graficas variando el ton.
      for (double numnodos = NUM_NODOS_INICIAL ; numnodos <= NUM_NODOS_MAXIMOS ; numnodos += NUM_NODOS_PASO)     
        {
          // Asignamos el valor a las variables
          NS_LOG_INFO ("Numero de nodos: " << numnodos);
         
          //Bucle para el intervalo de confianza del 95%
          for (uint32_t z = 0; z < NUM_IC; z++ )
            {
              
              NS_LOG_INFO ("Iteracion IC: " << z+1);
              
              double retardos; //Almacenara el retardo cada vez que se llame a simulacion
              double correctos; //Contendra el porcentaje de paquetes correctos cada vez que se llame a simulacion
          
              /*Llamada a la funcion simulacion pasandole el numero de nodos, los tiempos de encendido y apagado
                la fuente, la tasa de envio, el tamaño del paquete, el tamaño de la cola,
                retardos y correctos para que guarde en ellas los valores.*/ 
              //El 1 es el tamaño de la cola que como se va a variar hemos puesto por ahora 1
	      uint32_t cont=0;
	      for(double ton=TON_BAJA ; ton<=TON_MAXIMO; ton+=TON_PASO)
		{
		  onTime -> SetAttribute("Mean", DoubleValue (ton));
		  offTime -> SetAttribute("Mean", DoubleValue (duracionSilencioClientesMedia));
		  simulacion(nClientesPorCentral, velEnlace, retEnlace, onTime, offTime,tasaEnvioCliente, sizePkt,1,probErrorBitClientesMedia,&retardos,&correctos);
		  
		  //Vamos almacenando los valores en las variables Average.
		  puntos1[cont].Update(correctos);
		  puntos2[cont].Update(retardos);
		  cont++;
		}
            }
	  for(uint32_t contador2=0; contador2 < 3; contador2++)
	    {
	      media1[contador2] = puntos1[contador2].Avg(); //hacemos la media
	      varianza1[contador2] = puntos1[contador2].Var(); //Calculamos la varianza
	      IC1[contador2] = IC95*sqrt(varianza1[contador2]/NUM_IC); //Realizamos el IC

	      media2[contador2] = puntos2[contador2].Avg(); //hacemos la media
	      varianza2[contador2] = puntos2[contador2].Var(); //Calculamos la varianza
	      IC2[contador2] = IC95*sqrt(varianza2[contador2]/NUM_IC); //Realizamos el IC

	      //Asociamos a cada curva el IC 
	      curvagrafica1[contador2].Add(numnodos,media1[contador2],IC1[contador2]); 
	      curvagrafica2[contador2].Add(numnodos,media2[contador2],IC2[contador2]);
	      puntos1[contador2].Reset(); //Reseteamos los valores guardados en puntos.
	      puntos2[contador2].Reset(); //Reseteamos los valores guardados en puntos.
            }
        }      
      //Asociamos las curvas a cada grafica.
      for(uint32_t contador3=0; contador3 <3 ;contador3++)
	{
	  grafica1[contador3].AddDataset(curvagrafica1[contador3]);
	  grafica2[contador3].AddDataset(curvagrafica2[contador3]);
	  contador++;
	}
    }

  //Creamos los rotulos para las dos graficas.
  std::ostringstream rotulografica1;
  std::ostringstream rotulografica2;
  std::ostringstream rotulografica3;
  std::ostringstream rotulografica4;
  std::ostringstream rotulografica5;
  std::ostringstream rotulografica6;
  rotulografica1 << "Porcentaje de paquetes correctamente transmitidos en nivel bajo. sizePkt: " << sizePkt;
  rotulografica2 << "Porcentaje de paquetes correctamente transmitidos en nivel normal. sizePkt: " << sizePkt;
  rotulografica3 << "Porcentaje de paquetes correctamente transmitidos en nivel extremo. sizePkt: " << sizePkt;
  rotulografica4 << "Retardo medio en nivel bajo. sizePkt: " << sizePkt;
  rotulografica5 << "Retardo medio en nivel normal. sizePkt: " << sizePkt;
  rotulografica6 << "Retardo medio en nivel extremo. sizePkt: " << sizePkt;

  //Asociamos los rotulos a cada grafica.
  grafica1[0].SetTitle(rotulografica1.str());
  grafica1[1].SetTitle(rotulografica2.str());
  grafica1[2].SetTitle(rotulografica3.str());
  grafica2[0].SetTitle(rotulografica4.str());
  grafica2[1].SetTitle(rotulografica5.str());
  grafica2[2].SetTitle(rotulografica6.str());

  /*Guardamos la grafica1 en un fichero donde se representa el porcentaje de 
    paquetes correctamente transmitidos.*/ 
  grafica1[0].SetLegend("Numero de nodos.","Porcentaje paquetes correctos.[%]");
  std::ofstream fichero1("trabajo-1.1.plt");
  grafica1[0].GenerateOutput(fichero1);
  fichero1 <<" pause -1" << std::endl;
  fichero1.close();

 /*Guardamos la grafica1 en un fichero donde se representa el porcentaje de 
    paquetes correctamente transmitidos.*/ 
  grafica1[1].SetLegend("Numero de nodos.","Porcentaje paquetes correctos.[%]");
  std::ofstream fichero2("trabajo-1.2.plt");
  grafica1[1].GenerateOutput(fichero2);
  fichero2 <<" pause -1" << std::endl;
  fichero2.close();

 /*Guardamos la grafica1 en un fichero donde se representa el porcentaje de 
    paquetes correctamente transmitidos.*/ 
  grafica1[2].SetLegend("Numero de nodos.","Porcentaje paquetes correctos.[%]");
  std::ofstream fichero3("trabajo-1.3.plt");
  grafica1[2].GenerateOutput(fichero3);
  fichero3 <<" pause -1" << std::endl;
  fichero3.close();

  /*Guardamos la grafica2 en un fichero donde se representa el retardo medio.*/
  grafica2[0].SetLegend("Numero de nodos.","Retardo medio.[us]");
  std::ofstream fichero4("trabajo-2.1.plt");
  grafica2[0].GenerateOutput(fichero4);
  fichero4 << "pause -1" << std::endl;
  fichero4.close();

  /*Guardamos la grafica2 en un fichero donde se representa el retardo medio.*/
  grafica2[1].SetLegend("Numero de nodos.","Retardo medio.[us]");
  std::ofstream fichero5("trabajo-2.2.plt");
  grafica2[1].GenerateOutput(fichero5);
  fichero5 << "pause -1" << std::endl;
  fichero5.close();

  /*Guardamos la grafica2 en un fichero donde se representa el retardo medio.*/
  grafica2[2].SetLegend("Numero de nodos.","Retardo medio.[us]");
  std::ofstream fichero6("trabajo-2.3.plt");
  grafica2[2].GenerateOutput(fichero6);
  fichero6 << "pause -1" << std::endl;
  fichero6.close();

  return 0;
}
/*
bool
cumpleRequisitos ()
{
  // TODO
  return false;
}
*/

void
simulacion (uint32_t nClientesPorCentral, Ptr<ExponentialRandomVariable> velEnlace, 
	    Ptr<ExponentialRandomVariable> retEnlace, Ptr<ExponentialRandomVariable> onTime, 
	    Ptr<ExponentialRandomVariable> offTime, DataRate tasaEnvioCliente, uint32_t sizePkt,
	    uint32_t tamcola, double probErrorBitClientesMedia, double* retardos, double* correctos) {

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

}

