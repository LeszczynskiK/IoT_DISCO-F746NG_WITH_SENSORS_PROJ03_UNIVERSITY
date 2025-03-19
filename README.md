# IoT_DISCO-F746NG_WITH_SENSORS_PROJ03_UNIVERSITY

Projekt obejmuje:  
1. testowanie czujników IoT podłączonych do płytki DISCO-F746NG oraz ich programowanie
2. budowę komunikacji typu klient-serwer przy użyciu gniazd sieciowych
   
Celem projektu jest integracja czujników z mikrokontrolerem i wykorzystanie ich do pomiaru temperatury i ciśnienia.      
W celu rozszerzenia funkcjonalności zaimplementowano mechanizm serwera TCP,     
który został zmodyfikowany tak, aby użytkownik połączony z serwerem otrzymywał informację o temperaturze i ciśnieniu.  
Wartości te pobierane są z czujników.  

Zakres projektu:  
Podłączenie różnych czujników komunikujących się przez magistrale I2C i SPI(wykorzystam I2C).
Konfiguracja czujników i odczyt danych.
Testowanie działania poprzez wyświetlanie wyników na stronie www na localhost.

Sprzęt:  
Płytka: DISCO-F746NG    
Czujniki: STLM75, LPS331AP (wbudowane na płytce NUCLEO nałożonej na DISCO) 
Kompilator: Mbed OS  (Keil Studio Online)
Skrętka 
Router

Biblioteki:  
BSP_DISCO_F746NG  
EthernetInterface.h  
mbed.h  

Gdy uruchomiono, trzeba sprawdzić IP płytki(po połączeniu się do wifi w które jest ona wpięta): nmap -sn 192.168.1.0/24    
Gdy sprawdzono IP płytki, można w przeglądarce wpisac je i będzie widoczna podstawiona strona www    





