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

using namespace ns3;

class CalculoClientes
{
public:
  // Constructor de la clase
  CalculoClientes (char enlaceCentralesKbps[], char protocoloKbps[]);
  uint32_t GetInitialValue ();
  uint32_t GetValue ();
  uint32_t ResetValue ();
  bool FoundValue ();
private:
  uint32_t contadorClientes;
  uint32_t candidato;
  uint32_t iterador;
};

