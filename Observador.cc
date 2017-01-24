/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
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
  llamadasCorrectas = 0;
};

//Funcion asociada a la traza Tx, que almacena el tiempo de inicio del envio de un paquete
void Observador::ActualizaTinicio(Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  // Mostrar por salida debug el contenido del paquete
  std::ostringstream cad;
  paquete->Print (cad);
  NS_LOG_DEBUG (cad.str ());
  // Aumentamos el numero de paquetes enviados y almacenamos el tiempo inicial de envio
  numPaquetesEnviados++;
  NS_LOG_DEBUG ("Numero de paquetes enviados: " << numPaquetesEnviados);
  tiemposEnvios[paquete->GetUid ()]=Simulator::Now ();
  NS_LOG_DEBUG ("Paquete " << paquete->GetUid ()
                << " enviado en: "
                << Simulator::Now ().GetMilliSeconds () / 1000.0 << "s");
}

// Funcion asociada a la traza Rx, que calcula el retardo de un paquete
void Observador::ActualizaRetardos(Ptr<const Packet> paquete, const Address & direccion )
{
  NS_LOG_FUNCTION (paquete);
  // Mostrar por salida debug el contenido del paquete
  std::ostringstream cad;
  paquete->Print (cad);
  NS_LOG_DEBUG (cad.str ());
  // Buscar el paquete en la lista
  std::map<uint64_t, Time>::iterator indice;
  indice=tiemposEnvios.find(paquete->GetUid ());
  if(indice != tiemposEnvios.end())
  {
    // Si esta, se calcula el retardo, se almacena en el Average y se borra del mapa
    Time tinicio = tiemposEnvios[paquete->GetUid ()];
    Time tfinal = Simulator::Now();
    NS_LOG_DEBUG ("Paquete: " << paquete->GetUid () << " recibido en: " << Simulator::Now ().GetMilliSeconds () / 1000.0 << "s");
    // Calcular diferencia de tiempos y almacenar en Average de retardos
    double tiempo = tfinal.GetDouble () - tinicio.GetDouble ();
    NS_LOG_DEBUG ("Retardo paquete " << paquete->GetUid () << ": " << Time (tiempo).GetMicroSeconds () / 1000.0 << "ms");
    // Al haberse hecho uso del elemento, quitarlo de la lista
    tiemposEnvios.erase (indice);
    // Si se cumple que el retardo es menor a 140ms, aumentamos el contador de llamadas correctas
    if(tiempo < RETARDO_MAXIMO_US)
    {
      llamadasCorrectas++;
      retardos.Update (tiempo);
      NS_LOG_DEBUG("Numero de llamadas que cumplen la condicion de retardo: " << llamadasCorrectas);
    }
  }
  else
  {
    // Si no esta se muestra un mensaje tipo warning
    NS_LOG_WARN ("El paquete con UID " << paquete->GetUid () << " no esta en la estructura");
  }
}


// Funcion que devuelve la media de los retardos de los paquetes enviados 
Time Observador::GetMediaRetardos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediaRetardos;
  // Evitamos que se envie cuando una media es negativa y en su lugar enviamos un 0
  if (retardos.Avg () > 0)
  {
    mediaRetardos = retardos.Avg ();
  }
  else
  {
    mediaRetardos = 0;
    NS_LOG_WARN ("No se ha recibido ningun paquete: La media del retardo es 0");
  }
  return Time(mediaRetardos);
}


// Funcion que devuelve la media de los paquetes correctos
double Observador::GetMediaCorrectos()
{
  NS_LOG_FUNCTION_NOARGS ();
  double mediaCorrectos;

  if(numPaquetesEnviados == 0)
  {
    // Si no se han enviado pquetes, devolvemos un 0
    mediaCorrectos = 1;
    NS_LOG_WARN ("No se ha envido ningún paquete.");
  }
  else
  {
    // Si hay paquetes enviados, calculamos la cantidad de paquetes correctos
    mediaCorrectos = double (llamadasCorrectas) / numPaquetesEnviados;
  }
  NS_LOG_DEBUG ("Numero de paquetes totales enviados: " << numPaquetesEnviados);
  return 100 * mediaCorrectos;
}

// Funcion que compueba si la estructura esta vacia o no
void Observador::CompruebaEstructura ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG ("Tamanio de la estructura: " << tiemposEnvios.size ());
  //Comprobamos que la estructura no esta vacia
  if(tiemposEnvios.size () != 0)
  {
    NS_LOG_WARN ("Ha finalizado la simulacion y la estructura no esta vacia");
  }
}
