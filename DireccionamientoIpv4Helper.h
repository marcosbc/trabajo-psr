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
#include <ns3/data-rate.h>

#define IP_BASE "10"

using namespace ns3;

class DireccionamientoIpv4Helper
{
public:
  // Crea una subred y asigna IPs a los dispositivos conectados
  Ipv4InterfaceContainer CreateSubnet (uint32_t idCliente,
                                       NetDeviceContainer dispositivos);
  // Imprime la estructura de la red (util para debug)
  std::string ToString ();
  // Imprime las tablas de reenvio (util para debug)
  std::string RoutingTables ();
  // Obtener la IP de un determinado cliente, dentro de las subredes asignadas
  // Notese que una llamada requiere que dos clientes esten en la misma llamada
  Ipv4Address GetIp (uint32_t idCliente);
private:
  // Almacena el numero de subredes almacenadas, para asignamiento de nuevas subredes
  uint16_t subredesAsignadas;
};

