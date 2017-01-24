/*
 * TITULO: CalculaClientes
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Roldan
 * - Juan Lara Gavira
 * DESCRIPCION: Clase CalculaClientes, que se encarga de 
 *              calcular el numero de clientes optimo por central.
 */

#include "CalculoClientes.h"

NS_LOG_COMPONENT_DEFINE ("CalculoClientes");

/*
// VALIDACION PROPIA DEL FICHERO CALCULACLIENTES
//
// Inicio de test manual
// Se ha implementado una funcionalidad para probar el algoritmo de calculo
// de clientes, de forma separada a simulacion.cc, para lo cual es necesario
// que la constante TRABAJO no este definida.
#define NUM_CLIENTES_TEST 277
#define DEFAULT_TASA_CENTRALES "1Mbps"
#define REQUISITO_TASA_LLAM "64Kbps"
bool
cumpleRequisitos (uint32_t numClientes) {
NS_LOG_FUNCTION (numClientes);
if (numClientes > NUM_CLIENTES_TEST) {
return false;
}
return true;
}
int
main (void)
{
CalculoClientes instanciaCalculoClientes (
DEFAULT_TASA_CENTRALES, REQUISITO_TASA_LLAM
);
uint32_t maxNumClientes = instanciaCalculoClientes.GetInitialValue ();
while (! instanciaCalculoClientes.FoundValue ()) {
NS_LOG_DEBUG ("Iteracion: " << maxNumClientes << " clientes");
if (cumpleRequisitos (maxNumClientes)) {
NS_LOG_DEBUG ("Cumplimiento de requisitos con " << maxNumClientes << " clientes");
maxNumClientes = instanciaCalculoClientes.GetValue ();
} else {
NS_LOG_DEBUG ("No se cumplen los requisitos con " << maxNumClientes << " clientes");
// Incumple nodos, volver al valor anterior
maxNumClientes = instanciaCalculoClientes.ResetValue ();
}
}
return 0;
}
// Fin de test manual
*/

// Constructor de la clase
CalculoClientes::CalculoClientes ()
{
  // Definimos el valor inicial del contador de clientes
  // Queremos que sea divisible por 10, de ahi que se realice en dos pasos
  // Esto se consigue gracias al redondeo en C++ al usar tipos enteros
  contadorClientes = CALC_BASE;
  NS_LOG_DEBUG ("Valor inicial de contadorClientes: " << contadorClientes);
  // El candidato se inicia a cero y debe ser distinto al contador en la primera iteracion
  candidato = 0;
  // Iniciamos el iterador a cero
  iterador = 0;
  // Abortar en caso de que la primera iteracion falle
  abortar = false;
}

//Funcion que devuelve el numero base de la simulacion
uint32_t
CalculoClientes::GetInitialValue ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG ("Valor inicial de contador de clientes: " << contadorClientes);
  return CALC_BASE;
}

//Funcion que devuelve el valor del numero de clientes 
//actuales en cada iteracion
uint32_t
CalculoClientes::GetValue ()
{
  NS_LOG_FUNCTION_NOARGS ();
  candidato = contadorClientes;
  iterador++;
  contadorClientes = contadorClientes + (CALC_BASE * iterador);
  NS_LOG_DEBUG ("Valor de contador de clientes: " << contadorClientes);
  return contadorClientes;
}

//Funcion que va reseteando el valor del numero de clientes en funcion el numero base.
uint32_t
CalculoClientes::ResetValue ()
{
  NS_LOG_FUNCTION_NOARGS ();
  if (contadorClientes - candidato <= CALC_BASE)
    {
      contadorClientes = candidato;
    }
  else
    {
      // Aumentamos el
      contadorClientes = candidato + CALC_BASE;
      // Reseteamos el iterador
      iterador = 0;
    }
  NS_LOG_DEBUG ("Valor de contador de clientes tras reset: " << contadorClientes);
  return contadorClientes;
}

//funcino que encuentra el numero optimo de clientes por central
bool
CalculoClientes::FoundValue ()
{
  NS_LOG_FUNCTION_NOARGS ();
  bool result = abortar || (contadorClientes == candidato);
  if (abortar)
    {
      NS_LOG_INFO ("Abortando (contador de clientes: " << contadorClientes << ")");
    }
  else if (result)
    {
      NS_LOG_INFO ("Se ha encontrado optimo (contador de clientes: " << contadorClientes << ")");
    } else
    {
      NS_LOG_DEBUG ("No se ha encontrado optimo aun (contador de clientes: " << contadorClientes << ")");
    }
  return result;
}

//Funcion que aborta el algoritmo si es llamada
void
CalculoClientes::Abort ()
{
  NS_LOG_FUNCTION_NOARGS ();
  candidato = contadorClientes;
}

