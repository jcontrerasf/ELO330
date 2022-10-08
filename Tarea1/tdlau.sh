#! /bin/bash
# Obtengo los datos de dominio
last -w | awk '{print $3}' | while read line
do
    line2=${line//[0-9]/} # limpio los numeros posibles
    nchar=$(echo -n "${line2##*\.}" | wc -c) # cuento los caracteres del dom
    if test $nchar -gt 1
    then
      echo "${line2##*\.}" >> /tmp/t$$ # guardo los dominios validos
    fi
done
sort /tmp/t$$ | uniq -c >> /tmp/g$$ #cuenta la cantidad de accesos x dom
sort -k1 -n -r /tmp/g$$ #los ordena de mayor a menos n de accesos
rm /tmp/t$$
rm /tmp/g$$
# se eliminan los archivos termporales
#
