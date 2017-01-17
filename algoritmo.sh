#!/bin/bash

# Ejemplo: 64kbps con 1Mbps
ITERADOR_INI=2
CENTRAL_KBPS=1000
PROTOCOLO_KBPS=64
MARGEN_ERROR=$((2**$ITERADOR_INI))

numNodos=$((CENTRAL_KBPS/PROTOCOLO_KBPS))
iterador=$ITERADOR_INI
candidato=0

function simulacion() {
  echo $1
}

function cumple_con_requisitos() {
  echo "Comprobando $1..."
  if [ "$1" -ge 199 ]; then
    return 1
  fi
  return 0
}

while [ $numNodos -ne $candidato ]; do
  resultados=$(simulacion $numNodos)
  cumple_con_requisitos $resultados
  if [ $? -eq 0 ]; then
		candidato=$numNodos
    iterador=$((iterador+1))
  else
    diferencia=$((numNodos-candidato))
		if [ $numNodos -le $((candidato+MARGEN_ERROR)) ]; then
			break
		fi
    numNodos=$candidato
    iterador=$ITERADOR_INI
    # Para buscar solucion exacta:
		# if [ $candidato -eq $((numNodos-1)) ]; then
		#   break
		# fi
    # numNodos=$candidato
    # iterador=0
  fi
  numNodos=$((numNodos+2**iterador))
done

echo "Encontrado optimo: $numNodos"
