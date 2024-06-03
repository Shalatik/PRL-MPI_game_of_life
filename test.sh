#Simona Ceskova xcesko00
#24.04.2024
#PRL 2
#Implementace closed
#soubor pocita se vstupnim souborem, kterym je zakoncen jednim prazdnym radkem

#00000000
#00111000
#01110000
#00000000
#

#Pro vstupni soubor, ktery nema jeden prazdny radek (jako vyse), se vyhodnoti program jako segmentation for
#prosim o pripradnou upravu formatu vstupniho souboru


#!/bin/bash
numbers=$(wc -l < "$1")
procesors=$numbers
mpic++ --prefix /usr/local/share/OpenMPI -o life life.cpp
mpirun --prefix /usr/local/share/OpenMPI  -np $procesors life $1 $2