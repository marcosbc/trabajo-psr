/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include "LlamadasHelper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LlamadasHelper");

/*
// Inicio de test manual
#define NUM_CENTRALES 2
#define NUM_CLIENTES 170
#define T_START 0
#define T_STOP 100
#define PROB_LLAM 0.5
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
  // Asignar llamadas en la variable de registro
  // Notese que "randomIdCliente" genera valores de 0 a 2n - 1,
  // mientras que "idCliente" tendra un valor n
  uint32_t paresAsignados;
  uint32_t idCliente1 = 0, idCliente2 = 0;
  // Asignar la mitad de llamadas a clientes de la misma central
  // Del nodo 0 al n-1 estaran en central 1, de n a 2n-1 en central 2
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
  // Central 1: De n/4 a 3n/4 - 1
  // Central 2: De base + n/4 a base + n/2 - 1
  paresAsignados = 0;
  // NOTA: La division por 4 puede causar decimales, por eso se le
  // multiplica de nuevo por 2 para asegurarnos que todos los pares
  // quedan asignados
  while (paresAsignados < 2 * (numClientes / 4))
  {
    // Contar desde el inicio del tramo
    idCliente1 = numClientes / 2 + paresAsignados;
    // Contar desde el final del tramo
    idCliente2 = (3 * numClientes) / 2 - paresAsignados - 1;
    // Asignar la llamada
    asignarLlamada (idCliente1,
                    idCliente2,
                    duracionLlamadaValores,
                    tInicioLlamadaValores,
                    probLlamadaValores,
                    probLlamadaEnSimulacion);
    paresAsignados++;
  }
}

uint32_t
LlamadasHelper::GetIdDestino (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  return registroLlamadas[idCliente].idDestino;
}

Time
LlamadasHelper::GetStartTime (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
  return registroLlamadas[idCliente].startTime;
}

Time
LlamadasHelper::GetStopTime (uint32_t idCliente)
{
  NS_LOG_FUNCTION (idCliente);
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
    cadena << "* Cliente " << iter->first << ": "
           << "Dest: "<< (iter->second).idDestino << ", "
           << "inicioLlam: " << (iter->second).startTime.GetMilliSeconds () / 1000.0 << "s, "
           << "finLlam: " << (iter->second).stopTime.GetMilliSeconds () / 1000.0 << "s, "
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
  Time tInicio (0);
  Time tFin (0);
  // Comprobar si se produce llamada durante la simulacion
  if (probLlamadaValores->GetValue () >= probLlamadaEnSimulacion)
  {
    // Asignar los tiempo de inicio y fin
    tInicio = Seconds (tInicioLlamadaValores->GetValue ());
    tFin = operator+ (tInicio,
                      Seconds (duracionLlamadaValores->GetValue ()));
  }
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
  // Crear las entradas en los registros de llamada
  registroLlamadas[idCliente1] = llamadaCliente1;
  registroLlamadas[idCliente2] = llamadaCliente2;
}

