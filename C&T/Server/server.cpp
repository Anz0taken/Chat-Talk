#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <conio.h>
#include <ctime>

using namespace std;

#define 	PORTA 		1535	//porta utilizzata per stabilire la connessione
#define 	BUFSIZE		500  	//il numero massimi di byte per messaggio
#define 	MAXUSERS 	15		//numero massimo di utenti che si possono conettere
#define 	SENDSTATE 	300		//ogni quanto il server dovr� mandare un messaggio a ognio utente per farsi considerare online
#define 	TIMETOLIVE 	3000	//ogni utente dovr� farsi sentire prima di ogni "x" secondi altrimenti viene considerato disconnesso

/*=======================================================================
	Applicazione Chat
	Lato server
	Versione 3.0
	
	Questo software serve a gestire una sessione di chat tra massimo
	cinque utenti connessi su una stessa rete.
	Ci sar� una lobby, in qui ci sono tutti gli utenti appena connessi.
	E delle stanze, in cui dentro ci pu� essere un numero indefinito di
	utenti. Gli utenti potranno chattare solo quando saranno entrati
	in una stanza e solo con quelli nella stanza.
	
	Gli utenti connessi alla lobby saranno caricati in una lista gestita
	da un tipo di oggetto "GestoreLobby". Anche le stanze saranno gestite
	con le liste, gestite da un oggetto di tipo "GestoreStanze", dichiarato
	nell'oggetto di tipo "GestoreLobby". Inoltre, anche tutti gli utenti
	conenssi per ogni stanza saranno gestiti in delle liste, di cui si
	occupa direttamente l'oggetto di tipo "GestoreStanze".
	Infine, gli utenti in una stanza possono "Sfidarsi" tra di loro,
	avviando sessioni private di tris (tra due e soli due utenti).
	
		Luca Gargiulo							ITI medi 5Ai 29/06/19
-----------------------------------------------------------------------*/

/*============================================================================
	DIFFERENZA TRA CLIENT E SERVER
	
	+------------CLIENT-------------+----------------SERVER---------------+
	|  Creazione socket             |  Creazione socket                   |
	|  Connettersi al socket server |  Legare IP e porta al scoket        |
	|  Inviare e ricevere messaggi  |  Mettersi in ascolto                |
	|  Chiudere i socket            |  Accettare/Declinare le richieste   |
	|                               |  Inviare e ricevere messaggi        |
	|                               |  Chiudere i socket                  |
	+-------------------------------+-------------------------------------+
	
----------------------------------------------------------------------------*/

#include "Tris.h"

//Nodo di ogni utente conensso (sia alla lobby che alle stanze)
struct NODO{
	SOCKET 	Socket;
	int 	UltimoMessaggio;	//quand'� stato inviato l'ultimo messaggio
	int		Choosing;			//Viene utilizzata per determinare se: L'utente sta scegliendo il nome, l'utente sta scegliendo una stanza, l'utente � gi� in una stanza
	string 	IpHost;				//Ip dell'utente connesso al server
	string 	UserName;			//UserName dell'utente connesso al server
	string 	Avversario;			//l'utente contro cui sta giocando una partita
	string 	TipoGioco;			//il gioco che sta svolgengo con l'utente "Avversario"
	bool	Turno;				//turno del giocatore
	Tris 	*PartitaTris;
	NODO 	*Next;				//Puntatore al prossimo utente
};

//Nodo di ogni stanza presente nel server
struct NODO_Stanza{
	string 			NomeStanza;		//Nome della stanza
	string 			Password;		//Password della stanza
	int 			NumeroUsers;	//Numero di utenti connessi
	NODO			*UtentiConenssi;//Lista degli utenti connessi alla stanza
	NODO_Stanza 	*Next;			//Puntatore alla prossima stanza
};

//thread utilizzato per scrivere i messaggi senza interrompere la ricezione di messaggi dal server
DWORD WINAPI ScriviMessaggio(LPVOID lpParameter);

int flagthread = 0;	//variabile che tiene conto se il thread deve continuare o terminare

#include "Stanze.h"

//struttura utilizzata per permettere il passaggio multiplo di parametri nei diversi thread del progetto
struct A{
	SOCKET S;			//Socket dell'utente che sta scegliendo la stanza
	GestoreStanze *LS;	//Oggetto che gestisce le stanze
	char *Messaggio;	//Il messaggio che ha inviato
	NODO *TestaLista;	//La lista di tutti gli utenti nella lobby
	NODO *NodoUtente;	//Il nodo dell'utente
};

#include "Server.h"

main()
{
	SOCKET Socket, Utente;									//Socket del server									
	WORD VersioneWinSock;    								//16-bit unsigned integer
	WSADATA wsaData;
	struct sockaddr_in SockServer,SockClient; 				//Variabile contenente dati per la creazione del "legame" (bind())							
	GestoreLobby *Server;									//Oggetto che gestisce i socket degli utenti connessi
	
	char *Messaggio;										//Da debuggare ( se viene rimosso il programma crasha )
	Messaggio = new char[0];								//Da debuggare ( se viene rimosso il programma crasha )
	
	int *KeepLoop;											//punatore alla cella in cui ci sar� il valore che determina se il server dev'essere chiuso/"restare aperto"
	KeepLoop = new int;

	//Normale prassi
	VersioneWinSock = MAKEWORD( 2, 2);
	if(WSAStartup(VersioneWinSock, &wsaData))
	{ 
		cout<<"Errore nell'inizializzazione di WSA"<<endl;
		getch();
	}
  	else
  	{
  		//creazione socket -> protocol IPV4 / TCP / Best internet protocol
  		Socket = socket(AF_INET,SOCK_STREAM,0);
  		
		if(Socket<0)	//se il socket non � stato creato con successo
		{
			cout<<"Errore nell'inizializzazione del socket"<<endl;
			WSAGetLastError();
		}
		else
		{
			/*=============================================================================
				Ora dobbiamo associare il socket a un indirizzo IP e una porta
				con la funzione bind().
				
				sin_addr.s_addr indica quali connessioni accetter� l'applicazione
				(le costanti sono in formato macchina e quindi da convertire):
				
				INADDR_ANY      	indirizzo generico (0.0.0.0); connessioni 
				            		accettate da qualsiasi indirizzo associato al server
				
				INADDR_LOOPBACK   	indirizzo di loopback (127.0.0.1); possiamo  
				            		connetterci solo dalla macchina del server.
				
				INADDR_BROADCAST  	indirizzo di broadcast;
				
			------------------------------------------------------------------------------*/
			
			SockServer.sin_addr.s_addr = htonl(INADDR_ANY);				//accettiamo le connessioni da tutti gli indirizzi
			SockServer.sin_port        = htons(PORTA);					//settiamo la porta 1535
			SockServer.sin_family      = AF_INET;						//tipo di indirizzi che utilizza
			
			if( bind( Socket,(struct sockaddr *) &SockServer,sizeof(SockServer) ) < 0) //Se la creazione del "legame" � stato un insuccesso
			{
				cout<<"Errore creazione bind"<<endl;
				getch();
			}
			else
			{
			    char hostbuffer[256]; 
			    char *IPbuffer; 
			    struct hostent *host_entry; 

			    // To retrieve host information 
			    host_entry = gethostbyname(hostbuffer); 

			    // To convert an Internet network 
			    // address into ASCII string 
			    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 

			  	cout<<"======================SERVER  CHAT======================"<<endl;
			  	cout<<"Le persone della tua rete dovranno inserire questo IP : "<<IPbuffer<<endl;
			    cout<<"--------------------------------------------------------"<<endl;
			  	cout<<"> Premere enter per iniziare a mandare messaggi server <"<<endl;
			    cout<<"--------------------------------------------------------"<<endl;
			    
				u_long Argomento = 1;	//socket non bloccante
				
				//mettiamoci in ascolto
				listen(Socket, int(MAXUSERS));				//ci sar� "MAXUSERS" di persona in ascolto come massimo
				ioctlsocket(Socket, FIONBIO, &Argomento);	//socket non bloccante
				
				int Size = sizeof(struct sockaddr_in);
				
				*KeepLoop = 1;	//inizio ciclo polling
				
				Server = new GestoreLobby(MAXUSERS);
				
				int UserTTL = clock();
				
				while(*KeepLoop)	//ciclo di polling che avvia la sessione di chat
				{
					Utente = accept(Socket, (struct sockaddr*) &SockClient, &Size); //Alla socket "utente" vengono ora associati tutti i nuovi utenti che si connettono
					
					if(Utente != INVALID_SOCKET)		//se � il scoket � valido, quindi un nuovo utente si � connesso
						Server->AggiungiUsers(Utente,&SockClient);	//aggiungilo

					Server->RiceviMessaggi();						//Controlla l'arrivo di nuovi messaggi (LOBBY)
					Server->ListaStanze->RiceviMessaggiStanza();	//Controlla l'arrivo di nuovi messaggi (IN TUTTE LE STANZE)
					Server->UtentiAttivi();							//Controlla se qualche utente non � pi� online (LOBBY)
					Server->ListaStanze->UtentiAttivi();			//Controlla se qualche utente non � pi� online	(IN TUTTE LE STANZE)
					
					if(_kbhit())	//Se � stato premuto un tasto
						if(getch()==13)	//(enter)
							if(flagthread == 0)	//se il thread non � in esecuzione
							{
								flagthread = 1;		//Il server administrator sta scrivendo un messaggio
								
								DWORD Thread;
								HANDLE myHandle = CreateThread(0, 0, ScriviMessaggio, LPVOID(KeepLoop), 0, &Thread);	//create the thread
							}
					
					if(clock() - UserTTL > SENDSTATE)	//Se � arrivato il momento di mandare un messaggio a tutti gli utenti connessi al server informare che il server � ancora online
					{
						UserTTL = clock();							//Abbiamo appena mandato i messaggi
						Server->AggiornaClientTTL();				//Manda i messaggi a tutti gli utenti connessi nella lobby
						Server->ListaStanze->AggiornaCLientTTL();	// . . . server
					}
				}
			}

			closesocket(Socket);	//chiudiamo il socket creato
			WSACleanup();
		}
	}
	
	getch();
}

DWORD WINAPI ScriviMessaggio(LPVOID lpParameter)
{
	string Frase;
	
	cout<<"Inserire comando/messaggio : ";
	getline(cin,Frase);
	flagthread = 0;
	
	if(Frase == "/exit")
	{	
		//viene convertito prima LPVOID -> INT ... poi ... Da INT -> INT*
		int *Answer = (int*) int(lpParameter);
		*Answer = 0;
		cout<<"Chiudo connessioni . . ";
	}
	
	return 0;
}
