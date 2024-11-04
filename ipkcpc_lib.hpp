#include <stdbool.h>
#include <string.h>
#include <unistd.h> 
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <regex>
#include <sys/poll.h>
#include <cctype>
using namespace std;

enum
{
    PARAM_HIGH_E, //Velky pocet parametrov
    PARAM_LOW_E, //Maly pocet parametrov
    PARAM_E, //Neznamy/zly parameter

    PORT_E, //Chyba s cislom portu
    DNS_E, //Chyba kde zadany host neexistuje
    SOCKET_E, //Chyba pri vytvarani socketu
    CONNECT_E, //Chyba vytvorenia spojenia 

    SENDTO_E, //Chyba odoslania spravy serveru
    RECV_E,  //Chyba prijatia spravy od serveru
    OTHER_R //Ina chyba ktora zapricinila zastavenie programu
};