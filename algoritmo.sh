#!/bin/bash
numNodos=2
numNodosAnterior=1
iterador=1



function simulacion() {
  echo $1
}

function cumple_con_requisitos() {
  echo "Comprobando $1..."
  if [ "$1" -ge 129 ]; then
    return 0
  fi
  return 1
}



while [ $numNodosAnterior -ne $numNodos ]; do
  resultados=$(simulacion $numNodos)
  cumple_con_requisitos $resultados
  if [ $? -eq 1 ]; then
    numNodosAnterior=$numNodos
    iterador=$((iterador+1))
  else
    echo "NO CUMPLE"
    numNodos=$numNodosAnterior
    iterador=0
  fi
  numNodos=$((numNodos+iterador*2))
  echo "iterador=$iterador numNodosAnterior=$numNodosAnterior numNodos=$numNodos"
done