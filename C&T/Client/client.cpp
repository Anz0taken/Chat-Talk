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

#define 	PORTA 		1535	//Porta utilizzata per la connessione col server
#define  	BUFSIZE		500   	//il numero massimi di byte per messaggio
#define 	BEALIVE		200		//Ogni quanto il client manda un pacchetto al server per comunicargli di essere ancora in "vita"
#define 	MAXSERTTL	3000	//Dopo quanto il server viene considerato offline senza ricevere alcun messaggio

/*=======================================================================
	Applicazione Chat
	Lato client
	Versione 2.0
	
	Questo software serve a chattare con diversi utenti connessi a uno
	stesso server. Per poter chattare tra loro, gli utenti dovranno
	essere nella stessa "stanza".
	L'utente potrà scegliere un nome non utilizzato da alcun utente.
	Ogni stanza avrà un NOME e PASSWORD.
	L'utente avrà la lista stampata di tutte le stanze esistenti
	nel server dopo aver inserito un nome valido. Successivamente
	potrà scegliere se:
		inserire il nome e password di una stanza già esistente
	o
		creare una nuova stanza mettendo un nome inesistente,
		in questo caso gli verrà chiesto di creare anche una password.
	
	Fatto tutto, l'utente potrà iniziare a messaggiare solo con gli
	utenti connessi alla sua medesima stanza.
	
		Luca Gargiulo							ITI medi 5Ai 28/06/19
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

int flagthread = 0;	//variabile utilizzata per stabilire se: 0 L'utente non sta scrivendo alcun messaggio, 1 se lo sta scrivendo, 2 se è pronto per inviarlo
DWORD WINAPI myThread(LPVOID lpParameter);

struct CurPos
{
    CurPos():x(-1),y(-1) {}
    int x, y;
    operator bool() const { return x >= 0 && y >= 0; }
};

void Gotoxy(int x,int y);
CurPos getCursorPos();
CurPos DoveScrivere;			//variabile che si salva le cordinate del cursore quando bisogna inviare e ricevere nuovi messaggi

main()
{
	WORD VersioneWinSock;    								
	WSADATA wsaData;
	
	SOCKET Server,Utente;									//Socket Server/Utente
	struct sockaddr_in 	SockServer,SockClient; 				//Strutture dati che contengono le configurazioni di rete
	struct hostent      *HostName;
	unsigned short 		PortaServer;
	char 				Messaggio[BUFSIZE];
	
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
  		Server = socket(AF_INET,SOCK_STREAM,0);
  		
		if(Server<0)	//se il socket non è stato creato con successo
		{
			cout<<"Errore nell'inizializzazione del socket"<<endl;
			WSAGetLastError();
		}
		else
		{
			//--------------------------------------------------------------------
			// Chiede la connessione con il Server (che deve essere preparato 
			// per accettare le connessioni in arrivo): il kernel scegliere una 
			// porta effimera, con valore compreso tra 1024 e 5000. In questo caso
			// (socket TCP) la funzione connect avvia il three way handshake, e 
			// ritorna quando la connessione e' stabilita o si e' verificato errore.
			string IP;
			
			cout<<"Per favore, inserire un indirizzo IP a cui connettersi : ";
			getline(cin,IP);
			HostName = gethostbyname(IP.c_str());
			
			SockServer.sin_family = AF_INET;
			PortaServer           = PORTA;
			SockServer.sin_port   = htons(PortaServer);
			
			memcpy( &(SockServer.sin_addr), HostName->h_addr, HostName->h_length);
			
  			int Size = sizeof(struct sockaddr_in);
			
			while(connect(Server, (struct sockaddr*)&SockServer, Size))	//fino a quando non si è connessi al server
			{
				cout<<"--------------------------------------------------"<<endl;
				
				cout<<"Errore nella connessione del server"<<endl;
				cout<<"Per favore, inserire un indirizzo IP a cui connettersi : ";
				getline(cin,IP);
				HostName = gethostbyname(IP.c_str());
				memcpy ( &(SockServer.sin_addr), HostName->h_addr, HostName->h_length);
				
				cout<<"--------------------------------------------------"<<endl;
			}

			u_long Argomento = 1;
			ioctlsocket(Server, FIONBIO, &Argomento);	//socket non bloccante quando chiede se ha ricevuto un messaggio
			
			cout<<"Connessione riuscita . . ."<<endl;
			cout<<"--------------------------------------------------"<<endl;
			
			int SendTTL = clock();			//ogni "x" tempo il client manda un messaggio al server per indicargli di essere ancora in connessione 
			int ServerTTL = clock();		//per verificare se il server è ancora online -> se risponde
			string *MsgDaInviare = NULL;	//Puntatore string che eventualmente in futuro puntera a celle di memoria dedicate per l'input
			int Keep = 1;					//Fino a quando dev'essere online il client
			
			int Lobby;
			
			do	//ciclo di polling della chat
			{
				if(_kbhit())		//Se è stato premuto un tasto
					if(getch()==13)		//il tasto è enter
						if(flagthread == 0)	//e possiamo inviare un messaggio (perché il thread non è in esecuzione o perché alcun messaggio dev'essere già inviato)
						{
							flagthread = 1;				//segnaliamo che il thread è in esecuzione
							MsgDaInviare = new string;	//locazione spazio in cui ci sarà il messaggio
							
							DWORD Thread;
							HANDLE myHandle = CreateThread(0, 0, myThread, LPVOID(MsgDaInviare), 0, &Thread);	//crea il thread
						}
				
				if(flagthread == 2)		//Se il messaggio è pronto per essere inviato
				{
					flagthread = 0;								//Un eventuale nuovo thread può andare in esecuzione
					strcpy(Messaggio, MsgDaInviare->c_str() );	//copiamo il contenuto della stringa nell array di char
					Messaggio[MsgDaInviare->size()] = 0;		//mettiamo lo zero alla fine della frase
					send(Server, Messaggio, BUFSIZE, 0); 		//inviamo al server il messaggio
					delete MsgDaInviare;						//cancelliamo la memoria per il messaggio
				}
				
				if(clock() - SendTTL>BEALIVE)			//se dobbiamo comunicare al server di essere in vita
				{
					send(Server, "ALIVE", BUFSIZE, 0); 	//inviamo al server un messaggio
					SendTTL = clock();					//Aggiorniamo l'ultimo messaggio inviato
				}
				
				if(recv(Server,Messaggio,BUFSIZE,0)>0)	//Se il server mi ha mandato qualcosa
				{
					if(strcmp(Messaggio,"ALIVE")!=0)	//se il messaggio non è per riazzerare il TTL
					{
						if(flagthread==1)	//se l'utente sta scrivendo un messaggio
						{
							CurPos PosizioneAttuale;
							PosizioneAttuale = getCursorPos();				//ottieni la posizione attuale del cursore
							Gotoxy(0,DoveScrivere.y);						//vai al rigo in cui dev'essere mostrato il messaggio
							cout<<Messaggio<<endl;							//mostralo
							DoveScrivere.y++;								//sarà quindi il prossimo rigo in cui possiamo scrivere
							Gotoxy(PosizioneAttuale.x,PosizioneAttuale.y);	//ritorna al punto in cui l'utente stava scrivendo il messaggio
						}
						else
						{
							cout<<Messaggio<<endl;							//Mostralo
						}
					}
					
					//in ogni caso riaggiorna il TTL
					ServerTTL = clock();
				}
				
				if(clock() - ServerTTL > MAXSERTTL)
				{
					Keep = 0;
					cout<<"Il server ha smesso di rispondere . . ."<<endl<<"Controlla la tua connessione internet . . ."<<endl;
					getch();
				}
					
			}
			while(Keep);	//continua fino a quando il server non ci manda il messaggio "stop"
			
			closesocket(Server);	//chiudiamo il socket creato
			WSACleanup();
		}
	}
}

DWORD WINAPI myThread(LPVOID lpParameter)
{
	string *Frase = (string*) int(lpParameter);
	cout<<"Inserire comando/messaggio : ";
	DoveScrivere = getCursorPos();	//quando l'utente inizia a scrivere salvati le cordinate di dove scrive il messaggio
	DoveScrivere.y++;				//poichè l'attuale rigo è usato dall'utente per scrivere, quello sottostante sarà quello usato per mostrare nuovi messaggi
	getline(cin,*Frase);			//ottieni il messaggio dall'utente
	flagthread = 2;					//il messaggio è pronto e dev'essere inviato
	Gotoxy(0,DoveScrivere.y);		//sposta il cursore dov'è libero il rigo
 
	return 0;
}

void Gotoxy
(   int x,    // indice di colonna
	int y     // indice di riga
)
{	COORD Cur = {short(x),short(y)};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Cur);
}

CurPos getCursorPos()
{
    CurPos pos;
    CONSOLE_SCREEN_BUFFER_INFO con;
    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hcon != INVALID_HANDLE_VALUE &&
        GetConsoleScreenBufferInfo(hcon,&con)) 
    {
        pos.x = con.dwCursorPosition.X;
        pos.y = con.dwCursorPosition.Y;
    }
    return pos;
}
