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

LlamadasHelper::LlamadasHelper (uint32_t numClientes,
                                Ptr<RandomVariableStream> duracionLlamada,
                                Ptr<UniformRandomVariable> probLlamada)
{
  NS_LOG_FUNCTION (numClientes << duracionLlamada << probLlamada);
  std::map<uint32_t, uint32_t>::iterator iteradorClientes;
  // Asignar llamadas en la variable de registro
  // Notese que "randomIdCliente" genera valores de 0 a 2n - 1,
  // mientras que "idCliente" tendra un valor n
  uint32_t asignados = 0;
  uint32_t baseId = 0;
  uint32_t cliente1 = 0, cliente2 = 0;
  // Asignar la mitad de llamadas a clientes de la misma central
  for (uint32_t idCentral = 0; idCentral < NUM_CENTRALES; idCentral++)
  {
    // Del nodo 0 al n-1 estaran en central 1, de n a 2n-1 en central 2
    uint32_t baseId = idCentral * numClientes;
    // Asignar la mitad de los clientes (n) de central llamadas entre ellos
    while (asignados < numClientes / 4)
    {
      // Contar desde el inicio
      idCliente1 = baseId + asignados;
      // Contar desde el final
      idCliente2 = baseId + (numClientes / 2) - (asignados - 1);
      // Asignar la llamada
      asignarLlamada (idCliente1,
                      idCliente2,
                      duracionLlamada,
                      probLlamada);
      asignados++;
    }
  }
  // Asignar el resto a llamada a nodos de distintas centrales
  // Central 1: De n/4 a 3n/4 - 1
  // Central 2: De base + n/4 a base + n/2 - 1
  while (asignados < (3 * numClientes / 4))
  {
    idCliente1 = asignados;
    idCliente2 = baseId + (numClientes / 2) - (asignados - 1);
    asignados++;
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

void
LlamadasHelper::asignarLlamada (uint32_t idCliente1,
                                uint32_t idCliente2,
                Ptr<RandomVariableStream> duracionLlamadaValores,
                Ptr<UniformRandomVariable> tInicioLlamadaValores,
                Ptr<UniformRandomVariable> probLlamadaValores,
                double probLlamadaEnSimulacion)
{
  // Por defecto no habra llamada
  Time tInicio (0);
  Time tFin (0);
  // Comprobar si se produce llamada durante la simulacion
  if (probLlamadaValores.GetValue () >= probLlamada)
  {
    // Asignar los tiempo de inicio y fin
    tInicio = Seconds (tInicioLlamadaValores.GetValue ());
    tFin = operator+ (tInicio,
                      Seconds (duracionLlamadaValores.GetValue ()));
  }
  // Crear las entradas en los registros de llamada
  registroLlamadas[idCliente1] =
  {
    // Identificador del destino
    idCliente2,
    // Tiempo de inicio
    tInicio,
    // Tiempo de fin de llamada
    tFin
  };
  registroLlamadas[idCliente2] =
  {
    // Identificador del destino
    idCliente1,
    // Tiempo de inicio
    tInicio,
    // Tiempo de fin de llamada
    tFin
  };
}

