/*
 * TITULO: TODO
 * AUTORES:
 * - Desiree Garcia Soriano
 * - Marcos Bjorkelund
 * - Ana Lobon Garcia
 * - Juan Lara Gavira
 * DESCRIPCION: TODO
 */

#include "CalculoClientes.h"

NS_LOG_COMPONENT_DEFINE ("CalculoClientes");

// Constructor de la clase
CalculoClientes::CalculoClientes (char[] tasaCentrales, char[] tasaProto)
{
	uint32_t kbpsCentrales = (DataRate (tasaCentrales)).GetBitRate () / 1000;
	uint32_t kbpsProto = (DataRate (tasaProto)).GetBitRate () / 1000;
	// Definimos el valor inicial del contador de clientes
	// Queremos que sea divisible por 10, de ahi que se realice en dos pasos
	// Esto se consigue gracias al redondeo en C++ al usar tipos enteros
	contadorClientes = kbpsCentrales / (protocoloKbps * 10);
	contadorClientes = contadorClientes * 10;
	// El candidato se inicia a cero y debe ser distinto al contador en la primera iteracion
	candidato = 0;
	// Iniciamos el iterador a cero
	iterador = 0;
}

uint32_t
GetDefault ()
{
	return candidato;
}

uint32_t
GetValue ()
{
	candidato = contadorClientes;
	iterador++;
	contadorClientes = contadorClientes + (10 * iterador);
}

uint32_t ResetValue ()
{
	if (contadorClientes - candidato <= 10)
	{
		contadorClientes = candidato;
	}
	else
	{
		// Aumentamos el
		contadorClientes = candidato + 10;
		// Reseteamos el iterador
		iterador = 0;
	}
	return contadorClientes;
}

bool FoundValue ()
{
	return contadorClientes == candidato;
}

