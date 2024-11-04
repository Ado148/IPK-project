#include "ipkcpc_lib.hpp"
#define BUFSIZE 1024

//static volatile int execute = 1;
int client_socket = 0;
char buffer[BUFSIZE];

int Creating_connection(int udp_decision, struct hostent *server, char *IP_ADD, struct sockaddr_in server_addr, int PORT, int client_socket)
{
    if (udp_decision == 0)
    {
        // Ziskanie adresy serveru cez DNS
        if ((server = gethostbyname(IP_ADD)) == NULL)
        { // Prevod nazvu servera na IP adresu cez DNS
            fprintf(stderr, "Zadany host %s neexistuje.\n", IP_ADD);
            exit(DNS_E);
        }

        // Ziskanie adresy IP serveru, inicializacia struktury server_addr
        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET; // IPv4 adresa
        bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT); // Prevod portu na vhodny tvar

        // Vytvorenie socketu
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) <= 0) // protokol je vybrany automaticky
        {
            fprintf(stderr, "Chyba pri vytvarani socketu.\n");
            exit(SOCKET_E);
        }
        // Vytvorenie spojenia
        if (connect(client_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
        {
            fprintf(stderr, "Chyba pripojenia.\n");
            exit(CONNECT_E);
        }
    }
    else if (udp_decision == 1)
    {
        // Vytvorenie socketu
        if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) <= 0) // protokol je vybrany automaticky
        {
            fprintf(stderr, "Chyba pri vytvarani socketu.\n");
            exit(SOCKET_E);
        }
    }

    return client_socket;
}

void close_connection(int client_socket, int udp_decision)
{
    if (udp_decision == 0)
    {
        // uzavretie soketu pre dalsiu prijimanie dat
        shutdown(client_socket, SHUT_RD);
        // uzavretie soketu pre dalsiu komunikaciu
        shutdown(client_socket, SHUT_WR);
        // uzavretie soketu pre dalsiu komunikaciu a prijimanie dat
        shutdown(client_socket, SHUT_RDWR);
        close(client_socket);
    }
    else
    {
        close(client_socket);
    }
}

void Signal_handler(int handler)
{
    (void)handler; // Ostranenie warningu pre handler ktory sa nikde inde nepouziva
    string responseTCP = "BYE\n";
    int tcp_int_send = send(client_socket, responseTCP.c_str(), responseTCP.length(), 0);
    if (tcp_int_send < 0)
    {
        fprintf(stderr, "Chyba odoslania spravy.\n");
        exit(SENDTO_E);
    }

    bzero(buffer, BUFSIZE);
    int tcp_int_recv = recv(client_socket, buffer, BUFSIZE, 0);
    if (tcp_int_recv < 0)
    {
        fprintf(stderr, "Chyba prijatia spravy.\n");
        exit(RECV_E);
    }
    cout << buffer;

    close_connection(client_socket, 1);
    exit(0);
}

void Signal_handler_UDP(int handler)
{
    (void)handler; // Ostranenie warningu pre handler ktory sa nikde inde nepouziva
    close_connection(client_socket, 0);
    exit(0);
}

int main(int argc, char *argv[])
{
    int position = 0; // premenna, pomocou ktorej sa urcuje aktualna poloha daneho parametru v argv
    int args = 0;     // navratova hondota getopt, na zaklade ktorej sa pozotom vybera dany parameter pomocou switch
    char *IP_ADD = NULL;
    int PORT = 0;
    char *MODE = NULL;
    int udp_decision = 0; // pomocna premenna, kt. modifikuje funkciu Creating_connection podla zvoleneho modu
    int hello = 0;        // premenna, kt. urci ci sa vypise err v pripade ak prva sprava nie je HELLO

    int bytestx = 0, bytesrx = 0;
    struct hostent *server = 0;
    struct sockaddr_in server_addr;
    socklen_t serverlen;

    if (std::strcmp(argv[1], "-help") == 0 && argc == 2)
    {
        cout << "------------------------ Spustenie programu: ./ipkcpc -h <host> -p <port> -m <mode> ----------------------" << endl;
        cout << "----- Na poradi zadanych argumentov nezalezi. -m <mode> parameter sa zadava vo forme TCP/tcp UDP/udp -----" << endl;
        return 0;
    }

    if (argc > 7)
    {
        fprintf(stderr, "Zadali ste prilis vela parametrov, je potrebne zadat: ./ipkcpc -h <host> -p <port> -m <mode>, pre viac informacii zadajte ./ipkcpc -help\n");
        return PARAM_HIGH_E;
    }
    else if (argc < 6)
    {
        fprintf(stderr, "Zadali ste prilis malo parametrov, je potrebne zadat: ./ipkcpc -h <host> -p <port> -m <mode>, pre viac informacii zadajte ./ipkcpc -help\n");
        return PARAM_LOW_E;
    }
    else
    {
        while ((args = getopt(argc, argv, "h:p:m:")) != -1) //: vyzaduje argument k napr. -h treba IP adresu, : definuje ze h p m maju za sebou parameter
        {
            position++;
            switch (args)
            {
            case 'h':
                position++;
                IP_ADD = optarg;
                break;

            case 'p':
                position++;
                PORT = strtol(argv[position], NULL, 0); // Konvertuje string na int
                break;

            case 'm':
                position++;
                MODE = argv[position];
                break;

            default:
                fprintf(stderr, "Neznamy alebo zle zadany parameter, pre viac informacii zadajte ./ipkcpc -help\n");
                return PARAM_E;
            }
        }
    }

    if (PORT < 0 || PORT > 65535)
    {
        fprintf(stderr, "Zadany port moze byt len v rozsahu 0-65535.\n");
        return PORT_E;
    }
    char ip_check = *IP_ADD; // Kontrola spravnosti IPv4 adresy
    if (isdigit(ip_check))
    {
        if (!std::regex_match(IP_ADD, std::regex("^((25[0-5]|(2[0-4]|1[0-9]|[1-9]|)[0-9])(.(?!$)|$)){4}$")))
        {
            fprintf(stderr, "Zadana IP adresa je invalidna.\n");
            return OTHER_R;
        }
    }

    if ((strncmp(MODE, "tcp", 3) == 0) || (strncmp(MODE, "TCP", 3) == 0))
    {
        udp_decision = 0;
        client_socket = Creating_connection(udp_decision, server, IP_ADD, server_addr, PORT, client_socket);
        if (client_socket < 0)
        {
            fprintf(stderr, "Chyba vo funkcii vytvarania pripojenia.\n");
            return CONNECT_E;
        }

        signal(SIGINT, Signal_handler); // Ak uzivatel stlaci ctrl+c na ukoncenie bezaceho procesu, uzavri spojenie so serverom a vystup z programu
        while (true)                    // Nekonecna slucka, ktoras nacita, a vypisuje spravy do doby nez uzivatel nezada klucove slovo BYE
        {
            std::string str1_i;
            std::string str2_o;
            getline(cin, str1_i);
            str2_o += str1_i; // data
            str2_o += '\n';   // novy riadok

            transform(str2_o.begin(), str2_o.end(), str2_o.begin(), ::toupper); // prevedenie uzivatelovho vstupu na velke pismena

            if ((str2_o.find("HELLO") != string::npos) && (hello != 1))
            { // kontrola ci je prva sprava HELLO
                hello = 1;
            }
            else if (hello != 1)
            {
                hello = 2;
            }

            bytestx = send(client_socket, str2_o.c_str(), str2_o.length(), 0);
            if (bytestx < 0)
            {
                fprintf(stderr, "Chyba odoslania spravy.\n");
                return SENDTO_E;
            }

            bzero(buffer, BUFSIZE);
            bytesrx = recv(client_socket, buffer, BUFSIZE, 0);
            if (bytesrx < 0)
            {
                fprintf(stderr, "Chyba prijatia spravy.\n");
                return RECV_E;
            }

            cout << buffer;

            if (hello == 2)
                break;

            if ((strncmp(buffer, "BYE", 3)) == 0)
            {
                break;
            }
        }

        close_connection(client_socket, udp_decision);
    }
    else if ((strncmp(MODE, "udp", 3) == 0) || (strncmp(MODE, "UDP", 3) == 0))
    {
        // Ziskanie adresy serveru cez DNS
        if ((server = gethostbyname(IP_ADD)) == NULL)
        { // Prevod nazvu servera na IP adresu cez DNS
            fprintf(stderr, "Zadany host %s neexistuje.\n", IP_ADD);
            return DNS_E;
        }

        // Ziskanie adresy IP serveru, inicializacia struktury server_addr
        bzero((char *)&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET; // IPv4 adresa
        bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(PORT); // Prevod portu na vhodny tvar

        udp_decision = 1;
        client_socket = Creating_connection(udp_decision, server, IP_ADD, server_addr, PORT, client_socket);
        if (client_socket < 0)
        {
            fprintf(stderr, "Chyba vo funkcii vytvarania pripojenia.\n");
            return CONNECT_E;
        }

        serverlen = sizeof(server_addr);
        signal(SIGINT, Signal_handler_UDP); // Ak uzivatel stlaci ctrl+c na ukoncenie bezaceho procesu, uzavri soket
        while (true)                        // Nekonecna slucka, ktoras nacita, a vypisuje spravy do doby nez uzivatel nezada klucove slovo BYE
        {
            std::string str1_i;
            std::string str2_o;
            getline(cin, str1_i);
            
            if (std::cin.eof()) //Kontrola pritomnosti EOF
            {
                break;
            }

            int len = str1_i.length();
            str2_o += '\0';   // opcode
            str2_o += len;    // dlzka dat
            str2_o += str1_i; // data

            if (str2_o.length() > 255)
            {
                fprintf(stderr, "Sprava presiahla dlzku 8 bitov.\n");
                continue;
            }

            bytestx = sendto(client_socket, str2_o.c_str(), str2_o.length(), 0, (struct sockaddr *)&server_addr, serverlen); // Odoslanie spravy
            if (bytestx < 0)
            {
                fprintf(stderr, "Chyba odoslania spravy.\n");
                return SENDTO_E;
            }

            struct timeval timeout = {10, 0};                                                             // timeout na 10 sekund
            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)); // nastavemie timeout (10s) pre prijatie spravy

            bzero(buffer, BUFSIZE);
            bytesrx = recvfrom(client_socket, buffer, BUFSIZE, 0, (struct sockaddr *)&server_addr, &serverlen);
            if (bytesrx < 0)
            {
                fprintf(stderr, "Chyba prijatia spravy - vyprsanie casu na prijatie spravy.\n");
                return RECV_E;
            }

            int opcode_o = buffer[0];
            int status_o = buffer[1];
            int len_o = buffer[2];

            if (opcode_o == 1) // Skontroluj ci prisla odpoved a nie nieco ine
            {
                if (status_o == 1)
                {
                    cout << "ERR:" << &buffer[3] << &buffer[3 + len_o] << endl;
                }
                else if (status_o == 0)
                {
                    cout << "OK:" << &buffer[3] << &buffer[3 + len_o] << endl;
                }
            }
            else if (opcode_o == 0)
            {
                continue;
            }
        }
        close_connection(client_socket, udp_decision);
    }
    else
    {
        fprintf(stderr, "Program podporuje len mody TCP/UDP.\n");
        return OTHER_R;
    }
    return 0;
}