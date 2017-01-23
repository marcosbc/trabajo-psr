/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include "LlamadasHelper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LlamadasHelper");

/*
// Inicio de test manual
#define NUM_CENTRALES 2
#define NUM_CLIENTES 100
#define T_START 0
#define T_STOP 100
#define PROB_LLAM 0.2
#define DURACION_LLAM_MEDIA 60
int
main(void)
{
  // Crear variables aleatorias
  // Duracion de llamada
  Ptr<ExponentialRandomVariable> duracionLlamadaValores =
    CreateObject<ExponentialRandomVariable> ();
  duracionLlamadaValores->SetAttribute ("Mean", DoubleValue (DURACION_LLAM_MEDIA));
  // Tiempo de inicio de llamada
  Ptr<UniformRandomVariable> tInicioLlamadaValores =
    CreateObject<UniformRandomVariable> ();
  tInicioLlamadaValores->SetAttribute ("Min", DoubleValue (T_START));
  tInicioLlamadaValores->SetAttribute ("Max", DoubleValue (T_STOP));
  // Probabilidad de llamada durante la simulacion
  Ptr<UniformRandomVariable> probLlamadaValores =
    CreateObject<UniformRandomVariable> ();
  probLlamadaValores->SetAttribute ("Min", DoubleValue (0));
  probLlamadaValores->SetAttribute ("Max", DoubleValue (1));
  // Inicializar el helper
  LlamadasHelper llamadas (NUM_CLIENTES,
                           duracionLlamadaValores,
                           tInicioLlamadaValores,
                           probLlamadaValores,
                           PROB_LLAM);
  NS_LOG_INFO (llamadas.GetAsignacion ());
  return 0;
}
// Fin de test manual
*/

LlamadasHelper::LlamadasHelper (uint32_t numClientes,
                                Ptr<RandomVariableStream> duracionLlamadaValores,
                                Ptr<UniformRandomVariable> tInicioLlamadaValores,
                                Ptr<UniformRandomVariable> probLlamadaValores,
                                double probLlamadaEnSimulacion)
{
  NS_LOG_FUNCTION (numClientes
                   << duracionLlamadaValores << tInicioLlamadaValores
                   << probLlamadaValores << probLlamadaEnSimulacion);
  // Empezar inicializando a cero los atributos
  clientesAsignados.clear ();
  registroLlamadas.clear ();
  // Reservar espacio para la lista de id de clientes asignados
  for (uint32_t idCliente = 0; idCliente < 2 * numClientes; idCliente++)
  {
    // Por defecto no estan asignados
    clientesAsignados[idCliente] = false;
  }
  // Asignar llamadas en la variable de registro
  // Notese que "randomIdCliente" genera valores de 0 a 2n - 1,
  // mientras que "idCliente" tendra un valor n
  uint32_t paresAsignados;
  uint32_t idCliente1 = 0, idCliente2 = 0;
  // Asignar la mitad de llamadas a clientes de la misma central
  // Del nodo 0 al n-1 estaran en central 1, de n a 2n-1 en central 2
  // Central 1
  // Aproximadamente el 50% de sus nodos se comunicaran entre ellos
  paresAsignados = 0;
  while (paresAsignados < numClientes / 4)
  {
    idCliente1 = paresAsignados;
    idCliente2 = (numClientes / 2) - paresAsignados - 1;
    // Asignar la llamada
    asignarLlamada (idCliente1,
                    idCliente2,
                    duracionLlamadaValores,
                    tInicioLlamadaValores,
                    probLlamadaValores,
                    probLlamadaEnSimulacion);
    paresAsignados++;
  }
  // Central 2
  // Aproximadamente el 50% de sus nodos se comunicaran entre ellos
  paresAsignados = 0;
  while (paresAsignados < numClientes / 4)
  {
    idCliente1 = (3 * numClientes) / 2 + paresAsignados;
    idCliente2 = numClientes * 2 - paresAsignados - 1;
    // Asignar la llamada
    asignarLlamada (idCliente1,
                    idCliente2,
                    duracionLlamadaValores,
                    tInicioLlamadaValores,
                    probLlamadaValores,
                    probLlamadaEnSimulacion);
    paresAsignados++;
  }
  // Asignar el resto a llamada a nodos de distintas centrales
  // Solo para los nodos no asignados aun
  for (idCliente1 = 0; idCliente1 < numClientes * 2; idCliente1++)
  {
    // Obtener valor para idCliente1
    if (clientesAsignados[idCliente1])
    {
      // Volver a empezar la iteracion
      continue;
    }
    // Obtener valor para idCliente2
    for (idCliente2 = numClientes * 2 - 1; idCliente2 > 0; idCliente2--)
    {
      if (! clientesAsignados[idCliente2])
      {
        // Hemos encontrado un valor correcto, parar el bucle
        break;
      }
    }
    // En el caso de que ya esten asignados todos los clientes nos salimos
    if (clientesAsignados[idCliente1] || clientesAsignados[idCliente2])
    {
      break;
    }
    // Asignar la llamada
    asignarLlamada (idCliente1,
                    idCliente2,
                    duracionLlamadaValores,
                    tInicioLlamadaValores,
                    probLlamadaValores,
                    probLlamadaEnSimulacion);
  }
}

uint32_t
LlamadasHelper::GetIdDestino (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  NS_LOG_DEBUG ("idDestino = " << registroLlamadas[idCliente].idDestino);
  return registroLlamadas[idCliente].idDestino;
}

Time
LlamadasHelper::GetStartTime (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  NS_LOG_DEBUG ("startTime = " << registroLlamadas[idCliente].startTime);
  return registroLlamadas[idCliente].startTime;
}

Time
LlamadasHelper::GetStopTime (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  NS_LOG_DEBUG ("stopTime = " << registroLlamadas[idCliente].stopTime);
  return registroLlamadas[idCliente].stopTime;
}

std::string
LlamadasHelper::GetAsignacion ()
{
  std::map<uint32_t, DatosLlamada>::iterator iter;
  std::ostringstream cadena;
  cadena << "Asignaciones de llamadas:\n";
  // Recorrer todos los elementos en el registro de llamada
  for(iter = registroLlamadas.begin(); iter != registroLlamadas.end(); iter++)
  {
    // Imprimir los valores del registro de llamada
    cadena << "* clienteOrig " << iter->first << ": "
           << "clienteDest: "<< (iter->second).idDestino << ", "
           << "inicioLlam: " << (iter->second).startTime.GetMilliSeconds () / 1000.0 << "s, "
           << "finLlam: " << (iter->second).stopTime.GetMilliSeconds () / 1000.0 << "s"
           <<  "\n";
  }
  return cadena.str ();
}

void
LlamadasHelper::asignarLlamada (uint32_t idCliente1,
                                uint32_t idCliente2,
                Ptr<RandomVariableStream> duracionLlamadaValores,
                Ptr<UniformRandomVariable> tInicioLlamadaValores,
                Ptr<UniformRandomVariable> probLlamadaValores,
                double probLlamadaEnSimulacion)
{
  NS_LOG_FUNCTION (idCliente1 << idCliente2
                   << duracionLlamadaValores << tInicioLlamadaValores
                   << probLlamadaValores << probLlamadaEnSimulacion);
  // Por defecto no habra llamada
  Time tInicio ("1ms");
  Time tFin ("1ms");
  // Comprobar si se produce llamada durante la simulacion
  double comparadorProbLlam = probLlamadaValores->GetValue ();
  NS_LOG_DEBUG ("probLlam = " << probLlamadaEnSimulacion << ", "
                << "comparador = " << comparadorProbLlam);
  if (comparadorProbLlam <= probLlamadaEnSimulacion)
  {
    // Asignar los tiempo de inicio y fin
    tInicio = Seconds (tInicioLlamadaValores->GetValue ());
    tFin = operator+ (tInicio,
                      Seconds (duracionLlamadaValores->GetValue ()));
  }
  // Almacenar los nodos en el vector de llamadas asignadas

  // Definir las llamadas
  DatosLlamada llamadaCliente1 = {
    // Identificador del destino
    idCliente2,
    // Tiempo de inicio
    tInicio,
    // Tiempo de fin de llamada
    tFin
  };
  DatosLlamada llamadaCliente2 = {
    // Identificador del destino
    idCliente1,
    // Tiempo de inicio
    tInicio,
    // Tiempo de fin de llamada
    tFin
  };
  // Crear las entradas en los registros de llamada y aniadir a lista de
  // clientes con su llamada asignada
  // Cliente 1
  registroLlamadas[idCliente1] = llamadaCliente1;
  clientesAsignados[idCliente1] = true;
  // Cliente 2
  registroLlamadas[idCliente2] = llamadaCliente2;
  clientesAsignados[idCliente2] = true;
}

