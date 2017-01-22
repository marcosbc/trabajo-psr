
/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/application.h>
#include <ns3/average.h>
#include <ns3/data-rate.h>

using namespace ns3;

class Observador : public Application
{
public:

  Observador();

  // Metodos de la clase Observador

  //Funcion asociada a la traza:Tx, que va actualizando el tiempo de inicio del envio de un paquete.
  void ActualizaTinicio(Ptr<const Packet> paquete);

  //Funcion asociada a la traza:Rx que va calculando el retardo.
  void ActualizaRetardos(Ptr<const Packet> paquete, const Address & direccion );

  //Función que devuelve la media de los retardos producidos en el canal. 
  Time GetMediaRetardos();

  //Función que devuelve la media de los paquetes correctos.
  double GetMediaCorrectos();

  //Comprueba que la estructura no esta vacia
  void CompruebaEstructura();



private:

  // Variables de la clase Observador

  //Contendra el numero de paquetes enviados. 
   uint32_t numPaquetesEnviados;
  
  //Variable que almacenara los valores de los retardos.
  Average<double> retardos;

  //Estructura que almacenara los tiempos de envio de paquetes de todos los nodos.
  std::map<uint64_t, Time> tiemposEnvios;

  //Almacena el numero de llamadas correctas
  uint32_t llamadasCorrectas;

};
