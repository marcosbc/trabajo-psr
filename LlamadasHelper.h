/*
 * TITULO: LlamadasHelper
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: Definicion de la clase LlamadasHelper en donde se definen
 *              todas las funciones y variables principales que emplea 
 *              dicha clase.
 */

#include <ns3/core-module.h>
#include <ns3/random-variable-stream.h>

using namespace ns3;

//Estructura con los datos de la llamada.
typedef struct {
  // Identificador de cliente destino
  uint32_t idDestino;
  // Tiempo de inicio
  Time startTime;
  // Tiempo final de llamada
  Time stopTime;
} DatosLlamada;

class LlamadasHelper
{
 public:
  // Constructor: Crea una tabla de asignamiento de llamadas para clientes
  LlamadasHelper (uint32_t numClientes,
                  Ptr<RandomVariableStream> duracionLlamadaValores,
                  Ptr<UniformRandomVariable> tInicioLlamadaValores,
                  Ptr<UniformRandomVariable> probLlamadaValores,
                  double probLlamadaEnSimulacion);
  // Obtener el asignamiento de llamada para clientes
  // Notese que una llamada requiere que dos clientes esten en la misma llamada
  uint32_t GetIdDestino (uint32_t idCliente);
  // Obtener el tiempo absoluto de inicio de llamada
  Time GetStartTime (uint32_t idCliente);
  // Obtener el tiempo absoluto de fin de llamada
  Time GetStopTime (uint32_t idCliente);
  // Obtener en formato cadena la asignacion de nodos
  std::string GetAsignacion ();
 private:
  // Almacena la lista de nodos con llamada ya asignada
  std::map<uint32_t, bool> clientesAsignados;
  // Almacena las llamadas en curso, por cliente
  // Eso implica que para cada llamada habran dos registros, uno por cada
  // cliente final
  std::map<uint32_t, DatosLlamada> registroLlamadas;
  // Asignar una llamada entre dos clientes
  // Esto crea una entrada para los dos clientes, esten o no en la misma
  // central
  void asignarLlamada (uint32_t idCliente1,
                       uint32_t idCliente2,
                       Ptr<RandomVariableStream> duracionLlamadaValores,
                       Ptr<UniformRandomVariable> tInicioLlamadaValores,
                       Ptr<UniformRandomVariable> probLlamadaValores,
                       double probLlamadaEnSimulacion);
};

