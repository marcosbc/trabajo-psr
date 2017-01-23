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
#include <ns3/packet.h>
#include <ns3/callback.h>
#include <ns3/average.h>
#include <ns3/data-rate.h>
#include "Observador.h"
#include <ns3/ethernet-header.h>
#include <ns3/ipv4-header.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

Observador::Observador()
{
  // Inicializamos a 0 todos los contadores
  numPaquetesEnviados = 0;
  retardos.Reset();
  llamadasCorrectas = 0;

};

//Funcion asociada a la traza Tx, que almacena el tiempo de inicio del envio de un paquete
void Observador::ActualizaTinicio(Ptr<const Packet> paquete)
{
  /*
  NS_LOG_FUNCTION (paquete);
  Ptr<Packet> copia = paquete->Copy ();   // Obtener una copia del paquete
  EthernetHeader ethHeader;               // Cabecera de nivel de enlace
  Ipv4Header ipHeader;                    // Cabecera de nivel de red
  copia->RemoveHeader (ethHeader);        // Extraer la cabecera de nivel de red

  if(ethHeader.GetLengthType() == 0x0800) // Comprobar si es una trama para IP
  {
  	copia->RemoveHeader (ipHeader);     // Extraer la cabecera de IP
    if(ipHeader.GetProtocol() == 17)    // Comprobar si es un paquete UDP
    {
  */
    	//Es un paquete UDP, aumentamos el numero de paquetes enviados y almacenamos el tiempo inicial de envio
  		numPaquetesEnviados++;
  		NS_LOG_DEBUG ("Numero de paquetes enviados: " << numPaquetesEnviados);

  		tiemposEnvios[paquete->GetUid()]=Simulator::Now();
  		NS_LOG_DEBUG ("Paquete: " << paquete->GetUid()
                		<< " enviado en: "
               			<< Simulator::Now().GetMilliSeconds () / 1000.0 << "s");
  /*
  	}
  }
  */
}

//Funcion asociada a la traza Rx, que calcula el retardo de un paquete
void Observador::ActualizaRetardos(Ptr<const Packet> paquete, const Address & direccion )
{
  NS_LOG_FUNCTION (paquete);

  /*Ptr<Packet> copia = paquete->Copy ();   // Obtener una copia del paquete
  EthernetHeader ethHeader;               // Cabecera de nivel de enlace
  Ipv4Header ipHeader;                    // Cabecera de nivel de red
  copia->RemoveHeader (ethHeader);        // Extraer la cabecera de nivel de red

  if(ethHeader.GetLengthType() == 0x0800) // Comprobar si es una trama para IP
  {
  	copia->RemoveHeader (ipHeader);     // Extraer la cabecera de IP
   	if(ipHeader.GetProtocol() == 17)    // Comprobar si es un paquete UDP
    {
    	//Es un paquete UDP, buscamos en la estructura el tiempo inicial de envío del paquete
  */
    	std::map<uint64_t, Time>::iterator indice;
  		indice=tiemposEnvios.find(paquete->GetUid());

  		if(indice != tiemposEnvios.end())
    	{
      		//Si está, se calcula el retardo, se almacena en el Average y se borra del mapa
      		Time tinicio=tiemposEnvios[paquete->GetUid()];
      		Time tfinal = Simulator::Now();
      		NS_LOG_DEBUG ("Paquete: " << paquete->GetUid() << " recibido en: " << Simulator::Now().GetMilliSeconds () / 1000.0 << "s");
      		double tiempo;
      		tiempo = tfinal.GetDouble() - tinicio.GetDouble();
      		retardos.Update(tiempo);
      		NS_LOG_DEBUG ("Retardo paquete: "<< paquete->GetUid()<< " igual a: " << tiempo);
      		tiemposEnvios.erase(indice); 

      		//Si se cumple que el retardo es menor a 140ms, aumentamos el contador de llamadas correctas
      		if(tiempo < RETARDO_MAXIMO_US)
			{
	  			llamadasCorrectas++;
	  			NS_LOG_DEBUG("Numero de llamadas que cumplen la condicion de retardo: " << llamadasCorrectas);
			}
           
    	}
  		else //Si no esta se muestra un Warnig
    	{
      		NS_LOG_WARN ("Ha llegado un paquete que no esta en la estructura");
      		NS_LOG_DEBUG ("El uid del paquete que ha llegado es " << paquete->GetUid());
    	}
  /*
    }
  }
  */
}


//Funcion que devuelve la media de los retardos de los paquetes enviados 
Time Observador::GetMediaRetardos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediaretardos;
  // Evitamos que se envie cuando una media es negativa y en su lugar enviamos un 0
  if (retardos.Avg() > 0)
    mediaretardos=retardos.Avg();
  else
    {
      mediaretardos=0;
      NS_LOG_WARN ("No se ha recibido ninguna respuesta. La media de retardo es 0");    
    }

  return Time(mediaretardos);
}


//Funcion que devuelve la media de los paquetes correctos
double Observador::GetMediaCorrectos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediacorrectos;

  //Si no se han enviado pquetes, devolvemos un 0
  if((numPaquetesEnviados) == 0)
    {
      mediacorrectos=1;
      NS_LOG_WARN ("No se ha envido ningún paquete.");
    }
  //Si hay paquetes enviados, calculamos la cantidad de paquetes correctos
  else
    mediacorrectos=double(llamadasCorrectas)/numPaquetesEnviados;

  NS_LOG_DEBUG ("Numero de paquetes totales enviados: " << numPaquetesEnviados);
  return 100 * mediacorrectos;
}

//Funcion que compueba si la estructura esta vacia o no
void Observador::CompruebaEstructura()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG("Tamanio de la estructura: " << tiemposEnvios.size());
  //Comprobamos que la estructura no esta vacia
  if(tiemposEnvios.size()!=0)
    {
      NS_LOG_WARN ("Ha finalizado la simulacion y la estructura no esta vacia");
    }
}
