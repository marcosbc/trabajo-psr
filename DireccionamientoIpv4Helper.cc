/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include "DireccionamientoIpv4Helper.h"

NS_LOG_COMPONENT_DEFINE ("DireccionamientoIpv4Helper");

/*
// Inicio de test manual
// TODO
// Fin de test manual
*/

Ipv4InterfaceContainer
DireccionamientoIpv4Helper::CreateSubnet (NetDeviceContainer dispositivos)
{
  NS_LOG_FUNCTION (dispositivos);
  Ipv4AddressHelper direcciones;
  // Establecer la base
  // Notese que el tipo de "asignadas" es uint16_t, permitiendo almacenar
  // un maximo de 2^16 (65.536) subredes
  // Ademas haremos uso de las divisiones y restos por 256, de este valor,
  // para obtener las direcciones IP de las subredes
  direcciones.SetBase (IP_BASE
                       // Division por 256 para obtener primer octeto de red
                       << "." << subredesAsignadas / 256
                       // Resto de la division para obtener el segundo
                       << "." << subredesAsignadas % 256
                       // El ultimo octeto esta reservado para las interfaces
                       << ".0");
  // Aumentar el contador de direcciones asignadas
  // Notese que en la primera iteracion, el tercer y cuarto octeto son "0"
  asignadas++;
  // Devolver el contenedor de interfaces de red IPv4
  return direcciones.Assign (dispositivos);
}

std::string
DireccionamientoIpv4Helper::ToString ()
{
  NS_LOG_FUNCTION_NOARGS ();
  std::ostringstream result;
  // TODO
  return result.str ();
}

std::string
DireccionamientoIpv4Helper::RoutingTables ()
{
  NS_LOG_FUNCTION_NOARGS ();
  std::ostringstream result;
  // TODO
  return result.str ();
}

