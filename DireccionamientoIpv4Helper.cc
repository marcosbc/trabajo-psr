/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include "DireccionamientoIpv4Helper.h"

NS_LOG_COMPONENT_DEFINE ("DireccionamientoIpv4Helper");

/*
// Inicio de test manual
#define NUM_CENTRALES 2
#define NUM_CLIENTES 20
int
main(void)
{
  uint32_t numClientes = NUM_CLIENTES;

  // Creamos centrales
  NodeContainer centrales;
  centrales.Create (NUM_CENTRALES);
  // Definiciones: Enlaces y dispositivos de centrales
  PointToPointHelper enlacesCentrales;
  NetDeviceContainer dispositivosCentrales = enlacesCentrales.Install (centrales);

  // Creamos clientes
  NodeContainer clientes[NUM_CENTRALES];
  // Configurar enlaces y dispositivos con la central (punto a punto)
  // Notese que en total tendremos 2 * "numClientes" clientes
  // Cada central tendra "numClientes" clientes (parametro de simulacion ())
  NodeContainer* paresClienteCentral = new NodeContainer[2 * numClientes];
  PointToPointHelper* enlacesClienteCentral = new PointToPointHelper[2 * numClientes];
  NetDeviceContainer* dispClienteCentral = new NetDeviceContainer[2 * numClientes];
  // La logica de creacion de clientes es la misma en las dos centrales
  // Recorrer centrales y asociar nuevos nodos
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    clientes[idCentral].Create (numClientes + 1);
    // Asignar los pares cliente-central
    // De 0 a n-1 para la central 1, de n a 2n-1 para la central 2
    for (uint32_t iteradorClientes = 0;
         iteradorClientes < numClientes;
         iteradorClientes++) {
      // Lograr una iteracion desde 0 hasta 2n - 1
      uint32_t idCliente = iteradorClientes + idCentral * numClientes;
      // Aniadir central al par cliente-central
      paresClienteCentral[idCliente].Add (centrales.Get (idCentral));
      // Lo mismo para clientes (un cliente distinto por par central cliente)
      paresClienteCentral[idCliente].Add (clientes[idCentral].Get (iteradorClientes));
      // Instalar los dispositivos en los nodos
      dispClienteCentral[idCliente] =
        enlacesClienteCentral[idCliente].Install (paresClienteCentral[idCliente]);
    }
  }

  // Configuraciones de red
  // Instalamos la pila TCP/IP en todos los clientes y centrales
  InternetStackHelper pilaTcpIp;
  pilaTcpIp.Install (centrales);
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++) {
    pilaTcpIp.Install (clientes[idCentral]);
  }
  // Asignamiento de IPs, delegada a una clase
  DireccionamientoIpv4Helper direcciones;
  // Incluye subred para centrales
  Ipv4InterfaceContainer* subredes = new Ipv4InterfaceContainer[2 * numClientes + 1];
  // Red entre centrales (que es un poco especifica)
  subredes[0] = direcciones.CreateSubnet (numClientes * 2, dispositivosCentrales);
  for (uint32_t idCliente = 0; idCliente < 2 * numClientes; idCliente++) {
    // Notese que la subred #0 es la que conecta las centrales
    subredes[idCliente + 1] = direcciones.CreateSubnet (idCliente,
                                                        dispClienteCentral[idCliente]);
  }
  // Popular las tablas de enrutamiento, con el fin de que todos los clientes
  // de una central puedan acceder a los que pertenecen a esta, y tambien a los
  // clientes de la segunda central
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // Imprimir la estructura de la red
  NS_LOG_DEBUG (direcciones.ToString ());
  // Imprimir tablas de reenvio
  NS_LOG_DEBUG (direcciones.RoutingTables ());

  return 0;
}
// Fin de test manual
*/

DireccionamientoIpv4Helper::DireccionamientoIpv4Helper ()
{
  // Inicializarlos a cero
  subredesAsignadas = 0;
  interfacesParNodos.clear ();
}

Ipv4InterfaceContainer
DireccionamientoIpv4Helper::CreateSubnet (uint32_t idCliente,
                                          NetDeviceContainer dispositivos)
{
  NS_LOG_FUNCTION (idCliente << &dispositivos);
  Ipv4AddressHelper direcciones;
  // Notese que el tipo de "asignadas" es uint16_t, permitiendo almacenar
  // un maximo de 2^16 (65.536) subredes
  // Ademas haremos uso de las divisiones y restos por 256, de este valor,
  // para obtener las direcciones IP de las subredes
  std::ostringstream ipBase;
  ipBase << IP_BASE
         // Division por 256 para obtener primer octeto de red
         << "." << subredesAsignadas / 256
         // Resto de la division para obtener el segundo
         << "." << subredesAsignadas % 256
         // El ultimo octeto esta reservado para las interfaces
         << ".0";
  // Establecer la base
  direcciones.SetBase (Ipv4Address (ipBase.str ().c_str ()), IP_MASK);
  // Aumentar el contador de direcciones asignadas
  // Notese que en la primera iteracion, el tercer y cuarto octeto son "0"
  subredesAsignadas++;
  // Devolver el contenedor de interfaces de red IPv4 del par de nodos
  Ipv4InterfaceContainer interfaces = direcciones.Assign (dispositivos);
  // Asignar los valores
  interfacesParNodos[idCliente] = interfaces;
  // Devolver el contenedor de interfaces de red IPv4
  return interfaces;
}

std::string
DireccionamientoIpv4Helper::ToString ()
{
  NS_LOG_FUNCTION_NOARGS ();
  std::ostringstream result;
  // Obtener el ultimo par (interface conectado a enlace entre central)
  // No queremos mostrar las centrales en el asignamiento
  std::map<uint32_t, Ipv4InterfaceContainer>::iterator iter, centralesIter;
  if (! interfacesParNodos.empty()) {
    centralesIter = interfacesParNodos.end ();
    // Los dispositivos conectados al enlace entre centrales son los ultimos
    // elementos de "interfacesParNodos"
    centralesIter--;
  }
  // Imprimir asignacion de IPs de clientes
  result << "Asignacion de IPs de clientes:\n";
  for(iter = interfacesParNodos.begin(); iter != interfacesParNodos.end(); iter++)
  {
    // No imprimir el ultimo par (ya que son las interfaces entre centrales)
    if (iter != centralesIter)
    {
      result << "* Cliente " << iter->first << ": " << GetIp (iter->first) << "\n";
    }
  }
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

Ipv4Address
DireccionamientoIpv4Helper::GetIp (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  // Obtener la direccion del cliente (que no central)
  // Es la segunda interfaz del enlace
  return interfacesParNodos[idCliente].GetAddress (
    ID_CLIENTE_EN_CONTENEDOR_INTERFACES);
}

