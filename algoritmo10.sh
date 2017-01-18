#!/bin/bash

# Ejemplo: 64kbps con 1Mbps
ITERADOR_INI=0
CENTRAL_KBPS=1000
PROTOCOLO_KBPS=64

iterador=$ITERADOR_INI
candidato=0
numNodosDiv10=$((CENTRAL_KBPS/PROTOCOLO_KBPS/10))
numNodos=$((numNodosDiv10*10))

function simulacion() {
  echo $1
}

function cumple_con_requisitos() {
  echo "Comprobando $1..."
  if [ "$1" -ge 251 ]; then
    return 0
  fi
  return 1
}

while [ $numNodos -ne $candidato ]; do
  resultados=$(simulacion $numNodos)
  cumple_con_requisitos $resultados
  if [ $? -eq 1 ]; then
		candidato=$numNodos
    iterador=$((iterador+1))
    numNodos=$((numNodos+10*iterador))
  else
    diferencia=$((numNodos-candidato))
		if [ $diferencia -le 10 ]; then
			numNodos=$candidato
			break
		fi
    iterador=$ITERADOR_INI
		numNodos=$((candidato+10))
  fi
done

echo "Encontrado optimo: $numNodos"
