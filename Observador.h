
/*
 * TITULO: Observador
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: Definicion de la clase Observador, en el se definen todas las 
 *              funciones y variables a emplear por dicha clase.
 */

#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/application.h>
#include <ns3/average.h>
#include <ns3/data-rate.h>

#ifndef REQUISITO_LLAM_RETARDO_MAX
#define REQUISITO_LLAM_RETARDO_MAX "140ms"
#endif

using namespace ns3;

class Observador : public Application
{
 public:

  Observador();

  // Metodos de la clase Observador

  //Funcion asociada a la traza Tx, que almacena el tiempo de inicio del envio de un paquete
  void ActualizaTinicio (Ptr<const Packet> paquete);

  //Funcion asociada a la traza Rx, que calcula el retardo de un paquete
  void ActualizaRetardos (Ptr<const Packet> paquete, const Address & direccion);

  //Funcion que devuelve la media de los retardos de los paquetes enviados
  Time GetMediaRetardos ();

  //Funcion que devuelve la media de los paquetes correctos
  double GetMediaCorrectos ();

  // Deconstructor de la clase: Comprobaciones finales
  // En este caso se comprueba si la estructura de tiempos de envio esta
  // vacia, si no lo esta muestra un mensaje tipo Warning
  ~Observador ();

 private:

  // Variables de la clase Observador

  //Contiene el numero de paquetes enviados
  uint32_t numPaquetesEnviados;

  //Variable que almacena los valores de los retardos
  Average<double> retardos;

  //Estructura que almacena los tiempos de envio de paquetes de todos los nodos
  std::map<uint64_t, Time> tiemposEnvios;

  //Variable que almacena el numero de paquetes correctos
  uint32_t paquetesCorrectos;

};
