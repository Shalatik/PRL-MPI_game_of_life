# PRL-MPI_game_of_life

Hra Life

Hra Life reprezentuje příklad tzv. celulárního automatu. Hrací pole se skládá z buněk, které se v každém kroku přepínají mezi dvěma stavy:   

živá (značíme 1)  
mrtvá (značíme 0)  
Stavy buněk se v průběhu hry mění pomocí definované sady pravidel. Základní sada pravidel, kterou budete implementovat v projektu je následující:  

každá živá buňka s méně než dvěma živými sousedy umírá   
každá živá buňka se dvěma nebo třemi živými sousedy zůstává žít   
každá živá buňka s více než třemi živými sousedy umírá   
každá mrtvá buňka s právě třemi živými sousedy ožívá  


test.sh life.cpp {number of iterations}
