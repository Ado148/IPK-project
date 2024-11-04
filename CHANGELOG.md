## IPKCPC
Klientský program, ktorý zabezpečuje komunikáciu spolu s IPK Calculator protocol (IPKCP) so serverom. Klient podporuje dva módy:
- TCP (textový mód)
- UDP (binárny mód)

Príkazy číta zo štandartného vstupu, a odpoveď zo strany serveru vypisuje na štandatný výstup (STDOUT).

Preklad klienta na spustiteľný súbor sa vykonáva cez príkaz: ```make``` v koreňovom adresári, kde sa nachádza samotný ipkcpc.cpp. 

Samotného klienta spustíme príkazom :
 ```
 ./ipkcpc -h <host> -p <port> -m <mode> 
 ``` 
Kde ```-h <host>``` je IPV4 adresa daného serveru, ```-p <port>``` je port na ktorom beží server ku ktorému sa klient chce pripojiť (validný rozsah portov 0 - 65 535), ```-m <mode>``` je buď TCP/tcp alebo UDP/udp. Na poradí daných parametrov nezáleží.

Po zadaní parametrov v správnom formáte klientsky program, na základe zvoleného módu (TCP, UDP), začne posielať správy od užívateľa vzdialenému serveru na čo server bude odpovedať, dané správy od serveru sú potom vypisované na štandartný výstup (STDOUT). Ukončenie spojenia zo strany užívateľa v TCP móde nastáva ak užívateľ zadá slovo ```BYE```, ```C-с (CTRL + c)```, pripádne zle zadaný príkaz na výpočet alebo iné neznáme slovo na ktoré server automaticky odpovie ```BYE``` a ukončí spojenie. Ukončenie spojenia v UDP móde zo strany užívateľa sa môže vykonať prostredníctvom ```C-c (CTRL + c)```, v prípade že užívateľ zadá zle príklad server vypíše ```ERR:Could not parse the message``` avšak spojenie sa neprerušuje a užívateľ môže naďalej posielať správy.   
<p>&nbsp;</p>

## Známe problémy / nedostatky
### Problém/Nedostatok č. 1

V prípade že užívateľ zmení veľkosť vyrovnávacej pamäte (bufferu) v programe ```ipkcpc.cpp```  na príliš malú hodnotu napríklad ```#define BUFSIZE 10``` dôjde k tomuto: 
 ```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ a b)
ERR:Could n?
C^
 ```  
 V podstate ide o to že správa "pretiekla" cez vyrovnávaciu pamäť a zbytok správy sa už nezmestil a tak sa viac ako 10 znakov nevypísalo na výstup. Znak ```?``` značí neznáme miesto v pamäti. V takomto prípade sa dá pokračovať ďalej avšak v prípade, že výsledná správa presiahne opäť veľkosť vyrovnávacej pamäte, na výstupe sa objaví len časť danej správy od serveru.
<p>&nbsp;</p>

 ### Problém/Nedostatok č. 2 

 Ďalší problém je to, že referenčný server poskytnutý na otestovanie tohoto projektu pracuje so Signed char (-128, 127), čiže v prípade zadania správy dlhšej ako 127 znakov dochádza k ohláseniu chyby zo strany serveru.
  ```
 ./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ 1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 1)
ERR:Could not parse the message
 ```
 Avšak podľa priloženej dokumentácie k protokolu IPKCP by mal server podporovať "Payload Length" o veľkosti 8 bitov => až 255 znakov (2^8 = 255).

Očakávané chovanie je, že zo strany serveru by v prípade zadania správy o veľkosti viac ako 255 znakov sa mala vypísať chybová hláška, ktorá užívateľa upozorní na danú chybu a samozrejme užívateľ môže zadávať príklady na spracovanie ďalej.
 ```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ 100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 1)
ERR:Sprava presiahla dlzku 8 bitov.
 ```
<p>&nbsp;</p>

 ### Problém/Nedostatok č. 3

 Ďalší menší nedostatok, je že vstup napr.:
  ```
./ipkcpc -h 127.0.0.1 -m tcppp -p 2023
 ```
 je validný, tak isto ako aj
   ```
./ipkcpc -h 127.0.0.1 -m udppp -p 2023
 ```

 Avšak vstup 
   ```
./ipkcpc -h 127.0.0.1 -m tcccp -p 2023
Program podporuje len mody TCP/UDP.
 ```
 je invalidný a vyhodí program chybu, to isté platí aj pre UDP mód:
```
./ipkcpc -h 127.0.0.1 -m udddp -p 2023
Program podporuje len mody TCP/UDP.
 ```
<p>&nbsp;</p>

 ### Problém/Nedostatok č. 4

 Na platforme Windows daný program nefunguje, keďže program pracuje s knižnicami, ktoré Windows nepodporuje/nepozná. Avšak program bezproblémovo funguje na WSL2.0 (Windows Subsystem for Linux), na ktorom to bolo tiež testované. Avšak primárne to bolo testované na NixOS.  