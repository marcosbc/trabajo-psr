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

#define CALC_BASE 10

using namespace ns3;

class CalculoClientes
{
public:
  // Constructor de la clase
  CalculoClientes ();
  uint32_t GetInitialValue ();
  uint32_t GetValue ();
  uint32_t ResetValue ();
  bool FoundValue ();
  void Abort ();
private:
  uint32_t contadorClientes;
  uint32_t candidato;
  uint32_t iterador;
  bool abortar;
};

