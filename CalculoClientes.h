/*
 * TITULO: CalculaClientes
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: Definicion de la clase CalculoClientes, que contiene las 
 *              variables qu edicha clase emplea.
 * 
 */

#include <ns3/core-module.h>
#include <ns3/data-rate.h>

//Numero base de la simulacion, debe empezar por dicho numero 
//y ser divisible por el mismo
#define CALC_BASE 10

using namespace ns3;

class CalculoClientes
{
 public:
  // Constructor de la clase
  CalculoClientes ();
  
  //Funcion que devuelve el numero base de la simulacion
  uint32_t GetInitialValue ();
  
  //Funcion que devuelve el valor del numero de clientes 
  //actuales en cada iteracion
  uint32_t GetValue ();
  
  //Funcion que va reseteando el valor del numero de clientes 
  //en funcion el numero base.
  uint32_t ResetValue ();
  
  //funcino que encuentra el numero optimo de clientes por central
  bool FoundValue ();
  
  //Funcion que aborta el algoritmo si es llamada
  void Abort ();
 
 private:
  uint32_t contadorClientes;
  uint32_t candidato;
  uint32_t iterador;
  bool abortar;
};

