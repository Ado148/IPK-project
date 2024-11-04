# VUT FIT IPK

## Projekt 1

Klientský program, ktorý zabezpečuje komunikáciu spolu s IPK Calculator protocol (IPKCP) so serverom. Program číta príkazy zo štandartného vstupu (STDIN), a odpoveď zo strany serveru vypisuje na štandatný výstup (STDOUT). Chyby, ktoré by sa mohli počas behu klientského programu vyskytnúť sa vypisujú na štandartný chybový výstup (STDERR).

Tiež treba zdôrazniť, že daný protokol IPKCP pracuje v ABNF notácií. A samotné správy sú písané v prefixovej forme ako jeožné vidieť nižšie, kde sa riešia jednotlivé módy, ktoré klient podporuje.

Preklad klienta na spustiteľný súbor sa vykonáva cez príkaz: ```make``` v koreňovom adresári, kde sa nachádza samotný ipkcpc.cpp. 

Samotný klient sa spúšta príkazom :
 ```
 ./ipkcpc -h <host> -p <port> -m <mode> 
 ``` 
Kde ```-h <host>``` je IPV4 adresa daného serveru (od 0.0.0.0 do 255.255.255.255), ```-p <port>``` je port na ktorom beží server ku ktorému sa klient chce pripojiť (validný rozsah portov 0 - 65 535), ```-m <mode>``` je buď TCP/tcp alebo UDP/udp.

Príklady vstupu : 
 ```
 ./ipkcpc -h 127.0.0.1 -m tcp -p 2023
 ./ipkcpc -h 127.0.0.1 -m udp -p 2023
 ``` 
Pričom na poradí zadávaných argumentov (```-h <host> -p <port> -m <mode>```) nezáleží.

To že nezáleží na poradí daných argumentov je vďaka funkcií  ```getopt()``` a čítaču  ```position```. Funkcia ako parametre prijíma počet argumentov ```argc```, pole argumentov ```argv``` s ktorými bol program spustený a samotné parametre ktoré očakávame kde napr.: ```h:``` znamená, že za týmto prepínačom program očakáva ešte parameter. A toto sa vykonáva do doby než funkcia nevráti hodnotu -1. V samotných argumentoch sa potom pohybujeme cez ```switch``` na základe ktorého sa vyberajú jednotlivé argumenty tak ako boli zadané. V jednotlivých prípadoch ```case``` sa  potom parametre týchto argumentov uložia do premenných. Spomenutá premenná ```position``` sa používa na pohybovanie sa v poli argv, bez nej by sa totiž parametre daných argumentov ukladali do zlých premenných.

V prípade potreby je možné si nechať vypísať na STDOUT výpis o tom ako program spustiť a aké parametre používa prepínačom ```-help```.

Klient podporuje dva módy, ktoré sa vyberajú na základe zadaného parametru: ```-m <mode>```
- TCP (textový mód)
- UDP (binárny mód)
<p>&nbsp;</p>

V následujúcej sekcií sa bude testovať funkčnosť danej implementácie jak v TCP móde tak v UDP móde. Testovanie prebehlo v prostredí NixOS.

### TCP
Po zvolení TCP módu sa sa očákáva od užívateľa zadanie príkazu (správy) ```HELLO```, na čo server odpovie tiež ```HELLO```, po tomto sa prejde do ďalšieho stavu kde užívateľ môže zadávať rozličné príklady na vypočítanie. Samotné počítanie príkladov sa vynúti napísaním príkazu ```SOLVE``` za ktorým následuje zátvorka v ktorej sa nachádza daný priklad zapísaný v prefixovej notácií, napr.:```(+ 3 (* 5 2))```, čo vo výsledku vyhodí výsledok ```RESULT 13```. Je dôležité aby výraz bol zadaným tak ako je v tomto dokumente uvedené, inač dôjde k tomu, že server nepošle výsledok a vypíše len ```BYE```, na čo server aj klient každý uzvarie svoju čast spojenia. V prípade, že sa užívateľ rozhodne uzavrieť spojenie, može tak učiniť zadaním príkazu ```BYE```, na čo server aj klient každý uzvarie svoju čast spojenia.

Správy sa v tomto móde zasielajú v podobe veľkých písmen. Užívateľ však nemusí nutne všetko zadávať veľkými písmenami, keďže klient implicítne konvertuje všetky malé písmená na písmená veľké, to znamená, že vstup: ```solVe (+ 3 (* 5 2))``` je validným. 

Príklad správneho vstupu:
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
HELLO
HELLO
SOLVE (+ 8 (* 9 9))
RESULT 89
SOLVE (* 5 5)
RESULT 25
SOLVE (/ 5 5)
RESULT 1
BYE
BYE
```

Tiež validný vstup (case-insensitive). Server odpovedá vždy veľkými písmenami (tzv. uppercase).
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
heLLo
HELLO
Solve (+ 8 (* 9 9))
RESULT 89
bYe
BYE
```


Prvá správa nie je HELLO:
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
SOLVE (+ 8 (* 9 9))
BYE
```
Invalidný vstup s chýbajúcou zátvorkou, vyústí do toho, že server odpovie s ```BYE``` a ukončí svoju časť spojenia, tak isto aj klient správne ukončí spojenie. Server neodpovie na nami zadaný príklad keďže je z hľadiska syntaxe zle zapísaný.
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
HELLO
HELLO
SOLVE (+ 8 (* 9 9)
BYE
```

V prípade vynútenia ukončenia spojenia cez skratku (CTRL + c) dôjde k správnemu ukončeniu jak zo strany klienta tak zo strany serveru a server odpovie s ```BYE```.
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
HELLO
HELLO
SOLVE (- 100 50)
RESULT 50
C^
BYE
```

Zadanie nesprávneho príkazu má podobné následky ako u predošlého príkladu. Ako je možné opäť vidieť, server nám na daný príklad ```(+ 4 4)``` neodpovedal.
```
./ipkcpc -h 127.0.0.1 -m tcp -p 2023
HELLO
HELLO
SOLVE (+ 9 9)
RESULT 18
ADD (+ 4 4)
BYE
```

V prípade zadania portu mimo rozsah 0-65 536 dochádza k chybe.
 ```
./ipkcpc -h 127.0.0.1 -m tcp -p 65536
Zadany port moze byt len v rozsahu 0-65535.
 ``` 

Využitie DNS prekladu na IPV4 adresu serveru v TCP móde. Spustenie programu prebehlo s následnými argumentami  ```./ipkcpc -h merlin.fit.vutbr.cz -m tcp -p 10002 ```.
 ```
 ./ipkcpc -h 127.0.0.1 -m tcp -p 2023
HELLO
HELLO
solve (+ 9 9)
RESULT 18
^CBYE
 ```
<p>&nbsp;</p>

### UDP
V prípade zvolenia UDP módu užívateľ zadáva len príklady v podobe ```(* 5 5)```, na čo server odpovedá dvoma možnými spôsobmi
1. Variant: server vypíše správu spolu s výsledkom napr.: ```OK: 25```.
2. Variant: v prípade zle zadanej / formulovanej správy server vypíše ```ERR: <chybová hláška>```.

Pri UDP je dĺžka správy 8 bitov, to znamená že dĺžka správy môže byť maximálne 255 znakov. Tiež sa v tomto móde vyskytuje Timeout, ktorý po uplynutí 10 sekúnd, v prípade neprijatia správy zo strany serveru, uzavrie soket, tým pádom komunikácia sa preruší.

Príklad správneho vstupu:
```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ 4 (* 9 9))
OK: 85
```
Príklad chybného vstupu. Ako je možné vidieť v zadanej správe chýba ukončujúca zátvorka.
```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(-5 8
ERR:Could not parse the message
```

Keďže UDP je bezstavové narozdiel od TCP, ukončenie relácie medzi klientom a serverom je možné napríklad zadaním (CTRL + c), na čo sa uzavrie soket cez ktorý sa posielali datagramy. Za (CTRL + c) resp. C^ sa už ďalej nič neposiela a server a klient uzavreli spŕavne soket.
```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ 4 5)
OK: 9
C^
```

Ak užívateľ zadá delenie nulou, opäť server príde s chybovou hláškou avšak soket nezatvára a ďalej čaká na ďalšie príklady/správy.
```
./ipkcpc -h 127.0.0.1 -m udp -p 2023
(/ 1 0)
ERR:Could not parse the message
(+ 1 1)
OK: 2
```

V prípade zadania portu mimo rozsah 0-65535 dochádza k chybe.
 ```
./ipkcpc -h 127.0.0.1 -m udp -p 100000
Zadany port moze byt len v rozsahu 0-65535.
 ``` 

 Využitie DNS prekladu na IPV4 adresu serveru v UDP móde. Spustenie programu prebehlo s následnými argumentami  ```./ipkcpc -h merlin.fit.vutbr.cz -m udp -p 10002 ```.
 ```
 ./ipkcpc -h 127.0.0.1 -m udp -p 2023
(+ 1 1)
OK:2
(+ 87 2)
OK:89
(* 9 9)
OK:81
^C
 ```
<p>&nbsp;</p>

### Chybové kódy
Program pri rôznych inštanciach môže skončiť chybovým kódom, v tejto časti sa objasní čo jednotlivé chybové kódy znamenajú.
 ```
    0  - Veľký počet parametrov
    1  - Malý počet parametrov
    2  - Neznámy/zlý parameter
    3  - Chyba s číslom portu
    4  - Chyba kde zadaný host neexistuje
    5  - Chyba pri vytváraní socketu
    6  - Chyba vytvorenia spojenia 
    7  - Chyba odoslania správy serveru
    8  - Chyba prijatia správy od serveru
    9  - Iná chyba, ktorá zapríčinila zastavenie programu
 ```
 <p>&nbsp;</p>
 
### Teoretické východiská
"Táto kapitola bola prevzatá z [1] [2] [3]"

V rámci aplikácie IPKCPC sa využívajú dva protokoly pre komunikáciu so serverom. A to buď TCP alebo UDP. Oba spomenuté protokoly zaručujú prijímanie a odosielanie správ resp. dát.

Komunikácia v rámci sietí je zabezpečovaná typicky cez rozhrania, nazývané aj ako sokety (sockets) , cez ktoré odovzdáva aplikácia alebo nejaký proces v rámci operačného systému dáta transportnej vrstve a tiež prijíma dáta z tejto vrstvy. Môžme teda povedaž, že hlavnou úlohou transportnej vrstvy je zabepečiť prenos dát/správ medzi dvoma aplikáciami/procesmi na rôznych koncových zariadeniach. A transportná vrstva využíva sieťovú vrstvu na odoslanie/prijatie danej správy (na sieťovej vrstve paketu) na vzdialený počítač. Dáta odosielané/prijímané užívateľommôžu byť veľké aj niekoľko gigabajtov, preto je nutné aby ich transportná vrstva rozdelila na menšia časti. Každej tejto časti preradí hlavičku s informáciami pre transportnú vrstvu pijemncu, čím vziká segment. Sieťová vrstva  už ale vo forme paketu pošle dané dáta (pakety) na cieľovú stanicu. následne transportná vrstva cieľovej stanice z príchodzích segmentov vyberie dáta a pošle ich správnemu soketu, ktorý si otvorila nejaká aplikácia/proces.

V hlavičke kaž´deho segmentu sa nachádzajú dvojbatové čísla, ktoré reprezentujú cieľový a zdrojový port s hodnotami od 0- 65 535.

Keď aplikácia alebo nejaký iný proces otvára nový soket ten musí byť asociovaný s nejakým iným číslom na jednom alebo viacerých sieťových rozhraniach. Soket môže byť otovrený buď ako server alebo ako klient.

Ak sa soket otvorí ako serverový soket musí explicítne uviesť číslo portu s ktorým bude asociovaný, alebo mu môže operačný systém prideliť nejaký voľný port. Pochopiteľne každý segment poslaný na tento soket musí mať uvedený ako cieľový port toto číslo portu.

Klientsky soket musí pri otváraní daného soketu špecifikovať IP adresu cieľovej stanice a číslo cieľového portu, aby bolo jasné s akým soketom chce nadviazať komunikáciu. Okrem iného musí obsadiť nejaké číslo portu na svojom počítači (zdrojový port) čím úrči návratovú adresu pre aplikáciu/proces bežiacu na vzdialenom serveri, aby vedel kam má zaslať odpoveď.
<p>&nbsp;</p>

TCP (Transmission Control Protocol) je rozsiahlo používaný protokol v rámci transportnej vrstvy. TCP vytvára potvrdzovaný prenos dát medzi dvoma zariadeniami (len 1 ku 1), to znamená že všetky dáta z bodu A sa v rovnakom poradí dostanú do bodu B.
I keď TCP je považovaný za spoľahlivý protokol môže dôjsť k strate paketov, prípadne inej závažnej chybe počas prenosu, zvyčajne spôsobenej inou nižšou vrstvou, preto, v rámci TCP existujú mechanizmy na overenie toho, či prišli všetky pakety, či prišli v správnom poradí a či nie sú dáta, ktoré nesú poškodené. Ak sa zistí nejaký zo spomenutých problémov dôjde k zahodeniu týchto dát a vynúti sa ich opätovné odoslanie.
O TCP možno povedať, že je omnoho zložitejší ako UDP, tiež pomalší ale zato je spolahlivejší. PReto TCP sa zvyčajne používa v aplikáciach u ktorých je strata neakceptovateľná

UDP (User Datagram Protocol) je tiež masívne využívaným protokol transportnej vrstvy pre komunikáciu viacerých zariadení (1 ku N, kde N je ľubovoľný počet zariadení). 
Opäť protokoly nižších vrstiev nezaručujú spoľahlivý prenos každého segmentu do cieľového počítača, čiže segmenty UDP sa môžu stratiť počas cesty k príjemcovi alebo sú doručené v inom poradí (poprehadzované). to znamená, že UDP odovzdáva dáta aplikačnej vrstve v takom poradí v akom mu prišli narozdiel od TCP. 

Tiež je dôležíté si uvedomiť, že UDP nevytvára žiadny typ spojenia to znamená, že UDP je bezastavový protokol (nepotrebuje žiadny prechodný stav na uchovávanie informácií o jednotlivých spojeniach). O UDP je možné povedať, že je to vcelku jednoduchý protokol, ktorý je rýchly pretože mimo iné nemá žiadnu inicializáciu spojenia, ktorá by mohla spôsobiť oneskorenie, žiadne kontroly správnosti prijatých dát ani ich celistvosti. Preto sa UDP zvyčajne používa v streamovaní filmov/hier, on-line hrách kde občasná strata segmentov je tolerovateľná.
<p>&nbsp;</p>

### Zdroje
[1] Institute of Computer Science UPJS. 5. Prednáška – Transportná vrstva: Protokol TCP [online]. Košice: Institute of Computer Science UPJS. [cit. 17.03.2023]. Dostupné z: https://siete.ics.upjs.sk/prednaska-5/

[2] Institute of Computer Science UPJS. 4. prednáška
Transportná vrstva [online]. Košice: Institute of Computer Science UPJS. [cit. 17.03.2023]. Dostupné z: https://siete.ics.upjs.sk/wp-content/uploads/sites/4/2020/02/prednaska04.pdf

[3] Institute of Computer Science UPJS. 4. Prednáška – Transportná vrstva: UDP, úvod do potvrdzovaného prenosu dát [online]. Košice: Institute of Computer Science UPJS. [cit. 17.03.2023]. Dostupné z: https://siete.ics.upjs.sk/prednaska-4/

Fakulta informačních technologií VUT v Brně NES@FIT - Výzkumná skupina počítačové sítě. NESFIT IPK-Projekty [online]. Brno: NES@FIT - Výzkumná skupina počítačové sítě. [cit. 03.03.2023].Dostupné z: https://git.fit.vutbr.cz/NESFIT/IPK-Projekty/src/branch/master/Stubs/cpp

TCP Server-Client implementation in C [online]. GeeksforGeeks. [cit. 26.02.2023]. Dostupné z: https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

Catch Ctrl-C in C - Stack Overflow. Stack Overflow - Where Developers Learn, Share, & Build Careers [online]. Dostupné z: https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c

setting timeout for recv fcn of a UDP socket - Stack Overflow. Stack Overflow - Where Developers Learn, Share, & Build Careers [online]. Dostupné z: https://stackoverflow.com/questions/16163260/setting-timeout-for-recv-fcn-of-a-udp-socket

Fakulta informačních technologií VUT v Brně. 3 Programování síťových aplikací
IPK 2022/2023 L. Počítačové komunikace a sítě (IPK 22/23L) [online]. [cit. 17.03.2023]. Dostupné z: https://www.vut.cz/moodle



#### SCORE 18/18
