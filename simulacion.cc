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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Trabajo");

// Requisitos
#define REQUISITO_TASA_LLAM "64kbps"
#define REQUISITO_RETARDO_MAX "140ms"
#define REQUISITO_PORCEN_LLAM_CORRECTAS 99.99

void
simulacion ();

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Configuracion de requisitos y especificaciones
  DataRate tasaLlamMinima = REQUISITO_TASA_LLAM;
  Time retardoMaximo = REQUISITO_RETARDO_MAX;
  double porcenLlamadasCorrectasMin = REQUISITO_PORCEN_LLAM_CORRECTAS;

  // Configuracion de escenario
  uint32_t nClientesPorCentral = DEFAULT_NUM_CLIENTES;
  DataRate tasaCentrales = DEFAULT_TASA_CENTRALES;
  
  // Configuracion de clientes
  DataRate conexClientesMedia = DEFAULT_CLIENTES_CONEXION;
  Time retardoClientesMedia = DEFAULT_CLIENTES_RETARDO;
  uint32_t probErrorBitClientesMedia = DEFAULT_CLIENTES_PERROR_BIT;
  Time duracionLlamClientesMedia = DEFAULT_CLIENTES_DURACION_LLAMADA;
  Time duracionSilencioClientesMedia = DEFAULT_CLIENTES_SILENCIO;

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  simulacion ();
  

  // Valores predeterminados de valores
  uint32_t nCsma = DEFAULT_NCSMA;
  Time ton (DEFAULT_TON);
  Time toff (DEFAULT_TOFF);
  uint32_t sizePkt = DEFAULT_SIZEPKT;
  DataRate dataRate (DEFAULT_RATE);

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Dispositivos CSMA adicionales", nCsma);
  cmd.AddValue ("ton", "Valor medio inicial de tiempo de actividad", ton);
  cmd.AddValue ("toff", "Valor medio de tiempo de silencio", toff);
  cmd.AddValue ("sizePkt", "Tamanio de paquete", sizePkt);
  cmd.AddValue ("dataRate", "Tasa de bit en estado activo", dataRate);
  cmd.Parse (argc,argv);  
  
  
  

  // Siempre finaliza correctamente
  return 0;
}

void
simulacion () {

}

