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

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Time::SetResolution (Time::US);

  // Obtener parametros por linea de comandos
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // Siempre finaliza correctamente
  return 0;
}

