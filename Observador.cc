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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Observador");

Observador::Observador()
{
  // Inicializamos las variables.
   numPaquetesEnviados = 0;
  retardos.Reset();
  llamadasCorrectas = 0;

};

//Funcion asociada a la traza:Tx, que va actualizando el tiempo de inicio del envio de un paquete.
void Observador::ActualizaTinicio(Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  numPaquetesEnviados++;
  NS_LOG_DEBUG ("Numero de paquetes enviados: " << numPaquetesEnviados);
  //Introducimos en la estructura el tiempo inicial de envio.
  tiemposEnvios[paquete->GetUid()]=Simulator::Now();
  NS_LOG_DEBUG ("Paquete: " << paquete->GetUid()
                << " enviado en: "
                << Simulator::Now().GetMilliSeconds () / 1000.0 << "s");

}

//Funcion asociada a la traza:Rx que va calculando el retardo.
void Observador::ActualizaRetardos(Ptr<const Packet> paquete, const Address & direccion )
{
  NS_LOG_FUNCTION (paquete);
  std::map<uint64_t, Time>::iterator indice;
  indice=tiemposEnvios.find(paquete->GetUid());

  //Buscamos en la estructura el Uid. 
  if(indice != tiemposEnvios.end())
    {
      //Si está, se calcula el retardo, se almacena en el Average y se borra del mapa.

      Time tinicio=tiemposEnvios[paquete->GetUid()];
      Time tfinal = Simulator::Now();
      NS_LOG_DEBUG ("Paquete: " << paquete->GetUid() << " recibido en: " << Simulator::Now().GetMilliSeconds () / 1000.0 << "s");
      double tiempo;
      tiempo = tfinal.GetDouble() - tinicio.GetDouble();
      retardos.Update(tiempo);
      NS_LOG_DEBUG ("Retardo paquete: "<< paquete->GetUid()<< " igual a: " << tiempo);
      tiemposEnvios.erase(indice); 
      if(tiempo < 140000)
	{
	  llamadasCorrectas++;
	  NS_LOG_DEBUG("Numero de llamas que cumplen la condicion de retardo: " << llamadasCorrectas);
	}
           
    }
  else //Si no esta hacemos un Warnig
    {
      NS_LOG_WARN ("Ha llegado un paquete que no esta en la estructura");
      NS_LOG_DEBUG ("El uid del paquete que ha llegado es " << paquete->GetUid());
    }
   
}


//Función que devuelve la media de los retardos producidos en el canal. 
Time Observador::GetMediaRetardos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediaretardos;
  // Evitamos que se envie cuando una media es negativa y en su lugar enviamos un 0.
  if (retardos.Avg() > 0)
    mediaretardos=retardos.Avg();
  else
    {
      mediaretardos=0;
      NS_LOG_WARN ("No se ha recibido ninguna respuesta. La media de retardo es 0.");    
    }

  return Time(mediaretardos);
}


//Función que devuelve la media de los paquetes correctos.
double Observador::GetMediaCorrectos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediacorrectos;

  if((numPaquetesEnviados) == 0)
    {
      mediacorrectos=0;
      NS_LOG_WARN ("No se ha envido ningún paquete.");
    }
  else
    mediacorrectos=double(llamadasCorrectas)/numPaquetesEnviados;

  NS_LOG_DEBUG ("Numero de paquetes totales enviados: " << numPaquetesEnviados);
  return mediacorrectos;
}

//Funcion que compueba si la estructura esta vacia o no.
void Observador::CompruebaEstructura()
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_LOG_DEBUG("Tamanio de la estructura: " << tiemposEnvios.size());
  //Comprobamos que la estructura no esta vacia.
  if(tiemposEnvios.size()!=0)
    {
      NS_LOG_WARN ("Ha finalizado la simulacion y la estructura no esta vacia.");
    }
}


