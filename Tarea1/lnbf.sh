# !/bin/bash
N=${1:?"Falta un argumento obligatorio"}
dir=${2:-.} #por defecto trabaja sobre el directorio actual

cd $dir
ls -lSh "$PWD/"*.* 2> /dev/null | 
for i in $(seq 1 $N) #en las versiones mas recientes de bash se puede reemplazar $(seq... por {1..N}
do
  read line
  [[ -z $line ]] && { { [[ $i -eq 1 ]] && echo "Este directorio no tiene archivos" && break; } && continue; }
  echo $line | awk '{print $5, $9, $7, $6, $8}' #elemento 9 es el nombre del archivo
done
