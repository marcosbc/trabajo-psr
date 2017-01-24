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
#include <ns3/data-rate.h>
#include <ns3/internet-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/applications-module.h>
#include <ns3/ipv4-global-routing-helper.h>

#define IP_BASE "10"
#define IP_MASK "255.255.255.0"
#define ID_CLIENTE_EN_CONTENEDOR_INTERFACES 1

using namespace ns3;

class DireccionamientoIpv4Helper
{
public:
  // Inicializar valores de atributos
  DireccionamientoIpv4Helper ();
  // Crea una subred y asigna IPs a los dispositivos conectados
  Ipv4InterfaceContainer CreateSubnet (uint32_t idCliente,
                                       NetDeviceContainer dispositivos);
  // Imprime la estructura de la red (util para debug)
  std::string ToString ();
  // Obtener la IP de un determinado cliente, dentro de las subredes asignadas
  // Notese que una llamada requiere que dos clientes esten en la misma llamada
  Ipv4Address GetIp (uint32_t idCliente);
private:
  // Almacena el numero de subredes almacenadas, para asignamiento de nuevas subredes
  uint16_t subredesAsignadas;
  // Almacena la relacion "idCliente" - interfaces
  std::map<uint32_t, Ipv4InterfaceContainer> interfacesParNodos;
};

