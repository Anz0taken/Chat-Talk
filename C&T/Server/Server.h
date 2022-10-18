/*=======================================================================
	Quest'oggetto serve per gestire meglio tutti i socket degli utenti
	connessi. Il tipo di struttura dati utilizzata � la lista.
	Inoltre controlla se ci sono stati nuovi messaggi da tutti
	gli utenti connessi, se ci sono stati provvede a mandarli a tutti
	gli utenti tranne al mittente.
	
		Luca Gargiulo							ITI medi 5Ai 29/06/19
-----------------------------------------------------------------------*/

//classica struttura di un nodo

DWORD WINAPI ScegliStanza(LPVOID lpParameter);
DWORD WINAPI ScegliNome(LPVOID lpParameter);

class GestoreLobby{
	
	private:
		NODO 			*TestaLista;	//"Testa" della lista, viene direttamente dichiarato come nodo per facilitazione della scrittura codice
		int 			NumeroUsers;	//Numero di utenti connessi
		int 			MaxUsers;		//Massimo numero di utenti che si possono connettere
		
	public:
		GestoreStanze	*ListaStanze;	//Puntatore oggetto "GestoreStanze", come suggerisce il nome, serve a gestire le stanze
		
		GestoreLobby(int _MaxUsers)		//Costruttore
		{
			TestaLista = new NODO;
			TestaLista->Next = NULL;		//Non c'� alcun nodo
			int NumeroUsers = 0;			//Numero utenti uguale a zero
			MaxUsers = _MaxUsers;			//Definito il numero massimo di users
			ListaStanze = new GestoreStanze;//Creiamo un nuovo oggetto che gestisce tutte le stanze
		}
		
		//chiude i socket e rilascia tutta la memoria
		~GestoreLobby()
		{
			NODO *Pt1,*Pt2;
			
			for(Pt1 = TestaLista->Next; Pt1;)	//fino a che ci sono utenti
			{
				Pt2 = Pt1->Next;			//il secondo puntatore punta al secondo elemento
				closesocket(Pt1->Socket);	//viene rilasciato il socket dell'utente interessato
				delete Pt1;					//viene rilasciata la memoria
				Pt1 = Pt2;					//si passa al prossimo nodo
			}
		}
		
		bool AggiungiUsers(SOCKET SocketNuovoUser,struct sockaddr_in *SockUser)	//Aggiungi un nuovo user alla rete
		{		
			bool done = false;	//variabile usata per returnare un successo/insuccesso
			
			cout<<"Un utente sta tentando di connettersi . . ."<<endl;
			
			char *connected_ip = inet_ntoa(SockUser->sin_addr);
			
			cout<<"Indirizzo IP : "<<connected_ip<<endl;
			
			if(MaxUsers != NumeroUsers)	//se il numero di utenti connessi � minore del massimo numero supportato
			{
				NODO* Pt;
				for(Pt = TestaLista; Pt->Next!=NULL; Pt = Pt->Next);	//vai alla fine della lista degli user
				Pt->Next = new NODO;	//crea un nuovo nodo, questo sar� puntato dal "next" di quello che ora � il penultimo
				Pt = Pt->Next;			//spostiamoci sull'ultimo nodo
				Pt->Socket = SocketNuovoUser;	//mettiamo il nuovo socket nel nodo
				Pt->UltimoMessaggio = clock();
				Pt->IpHost = string(connected_ip);	//mettiamoci l'ip dell'utente connesso
				Pt->Choosing = 5;					//L'utente deve scegliere il suo nome
				Pt->PartitaTris = NULL;				//Il puntatore non punter� ad alcun oggetto che gestisce una sfida tris, verr� fatto quando ci sar� una sfida
				Pt->Turno = false;					//l'utente non sta giocando
				Pt->Next = NULL;					//l'ultimo nodo non ha next
				
				NumeroUsers++;	//� incrementato il numero di utenti connessi
				
				done = true;	//� stato aggiunto un utente in modo corretto
				
				cout<<"Conenssione riuscita"<<endl;
			}
			else
			{
				cout<<"Connessione negata"<<endl;
				closesocket(SocketNuovoUser);	//chiudiamo il socket creato
			}
			
			cout<<"--------------------------------------------------------"<<endl;
			return done;	//dai la risposta
		}
		
		void RiceviMessaggi()
		{
			NODO *Pt;
			
			char *Messaggio;
			Messaggio = new char[BUFSIZE];
			
			for(Pt = TestaLista->Next; Pt; Pt = Pt->Next)	//per ogni utente connesso alla lobby
			{
				/*==========================================================================
					Nel caso in cui l'utente st� scegliendo la stanza, non sar� pi� la
					funziona "RiceviMessaggi" a riaggiornare il TTL, sar� stesso il thread
					dedicato alla scelta della stanza "ScegliStanza()".
				--------------------------------------------------------------------------*/
				if(recv(Pt->Socket, Messaggio, BUFSIZE, 0)>0)	//se ha inviato un messaggio
				{
					Pt->UltimoMessaggio = clock();
					
					if(!Pt->Choosing)	//Se l'utente non sta scegliendo alcuna stanza, e ha gi� inserito il nome
					{	
						if(strcmp(Messaggio,"ALIVE")!=0) //se non � un messaggio di controllo
						{
							Pt->Choosing = true; //l'utente ora sta scegliendo una stanza
							
							//per ogni utente che vuole entrare crea un thread
							
							/*==========================================================================
								Poich� al thread si pu� passare una sola variabile di tipo LPVOID e noi
								abbiamo necessit� di passargli pi� variabili, creiamo una struct mettendoci
								dentro tutti i dati di cui abbiamo. Successivamente al thread passiamo
								il puntatore alla struct.
							--------------------------------------------------------------------------*/
							
							A *B;	//creiamo la struct
							
							B = new A;	//allochiamola sulla memoria heap
							
							//Assegnamogli il valore
							B->S = Pt->Socket;
							B->LS = ListaStanze;
							B->Messaggio = Messaggio;
							B->TestaLista = TestaLista;
							B->NodoUtente = Pt;
							
							DWORD Thread_;
							HANDLE myHandle = CreateThread(0, 0, ScegliStanza, LPVOID(B), 0, &Thread_);		//crea il thread
						}
					}
					else if(Pt->Choosing == 5)	//se deve ancora scegliere il nome
					{
						/*=================================================================
							Creare un thread dedicato alla selezione del nome.
							
							Al thread dobbiamo passare tutti i dati della struct A;
							
							Nel thread : 
							
								Cambiare subito Il valore Choosing (5) nel valore (6) del nodo
								del nostro utente.
								
								Restare in attesa affinch� l'utente non digiti un nome
								Quando l'ha digitato bisogna :
									Controlla se c'� un nome uguale nella lobby
										Se non c'�:
											Controlla se c'� un nome uguale in tutte le stanze
												Se non c'�:
													Assegna all'attributo UserName del nodo il nome voluto
													Assegna all'attributo Choosing del nodo il valore 0
													Rilasciare il puntatore di struct A
										Se c'� riprova dall'inizio
												Se c'� riprova dall'inizio
								
								Significato valori :
								5 = Deve ancora scegliere il nome
								6 = Sta scegliendo
								0 = L'utente deve scegliere una stanza
						-----------------------------------------------------------------*/
						
						Pt->Choosing = 6;	//l'utente sta scegliendo un nome
											
						A *B;	//creiamo la struct
						
						B = new A;	//allochiamola sulla memoria heap
						
						//Assegnamogli il valore
						B->S = Pt->Socket;
						B->LS = ListaStanze;
						B->Messaggio = Messaggio;
						B->TestaLista = TestaLista;
						B->NodoUtente = Pt;
						
						DWORD Thread_;
						HANDLE myHandle = CreateThread(0, 0, ScegliNome, LPVOID(B), 0, &Thread_);		//crea il thread
					}
				}
			}
			
			delete Messaggio;
		}
		
		/*==========================================================================
			Questa funzione serve a verificare che tutti gli utenti precedentemente
			connessi al server (controlla solo quelli nella lobby)
			lo siano ancora, nel caso contrario il socket viene
			chiuso e la memoria dedicata all'utente interessato rilasciata.
		--------------------------------------------------------------------------*/
		void UtentiAttivi()
		{
			NODO *Pt1,*Pt2;
			
			for(Pt1 = TestaLista, Pt2 = Pt1->Next; Pt2;Pt1 = Pt1->Next, Pt2 = Pt2->Next)	//fino a che ci sono utenti
			{
				if(clock() - Pt2->UltimoMessaggio>TIMETOLIVE)	//se l'utente non ci ha mandato alcun messaggio negli ultimi "x" secondi
				{
					cout<<"Un utente si e' disconnesso (timed out). . ."<<endl;
					cout<<"Nome : "<<Pt2->UserName<<"  IP : "<<Pt2->IpHost<<endl;
					cout<<"--------------------------------------------------------"<<endl;
					
					Pt1->Next = Pt2->Next;		//il nodo precedente (rispetto al nodo che vogliamo cancellare) punta al nodo successivo (rispetto al nodo che vogliamo cancellare)
					closesocket(Pt2->Socket);	//chiudiamo il socket creato
					delete Pt2;					//cancelliamo il nodo
				}
			}
		}
		
		/*==========================================================================
			Questa funzione serve a mandare un messaggio a tutti gli utenti
			della LOBBY connessi al fine di potergli segnalare che il server
			� ancora funzionante.
		--------------------------------------------------------------------------*/
		void AggiornaClientTTL()
		{
			NODO *Pt;
			
			//int Quanti = 0;
			
			//cout<<"Inizio inotro messaggio (AGGIORNAMENTO STATE) . . ."<<endl;
			//cout<<"Numero di destinatari raggiunti : ";
			
			for(Pt = TestaLista->Next; Pt; Pt = Pt->Next)	//Scorri tutta la lista degli utenti connessi nella lobby
			{
				send(Pt->Socket, "ALIVE", BUFSIZE, 0);	//manda il messaggio
				//Quanti++;
			}
						
			//cout<<Quanti<<endl;
			//cout<<"--------------------------------------------------------"<<endl;
		}
};

DWORD WINAPI ScegliStanza(LPVOID lpParameter)
{
	//I dati passano quindi al thread
	
	A *B;
	
	/*==========================================================================
		Un puntatore � un intero di 4 Byte.
		Abbiamo per� questo valore in una variabile LVOID.
		Convertiamola quindi da LPVOID a un INT,
		e da INT in un puntatore di tipo (A*).
	--------------------------------------------------------------------------*/
	B = (A*) int(lpParameter);
	
	SOCKET Utente;
	Utente = B->S;
	GestoreStanze *ListaStanze;
	ListaStanze = B->LS;
	char *Messaggio = B->Messaggio;
	NODO *ListaLobby = B->TestaLista;
	NODO *NodoUtente = B->NodoUtente;
	
	string NomeStanza = string(B->Messaggio);
	
	cout<<"Un utente sta scegliendo una stanza ("<<NodoUtente->UserName<<") . . ."<<endl;
	cout<<"--------------------------------------------------------"<<endl;
	
	/*=============================================================================================================
		Se l'utente ha inviato un messaggio alla lobby, significa che non � connesso in alcuna stanza privata.
		Per tanto diamo per scontato che i messaggi che invier� saranno per cercare delle stanze.
	--------------------------------------------------------------------------------------------------------------*/
	if( ListaStanze->CercaNomeStanza( NomeStanza ) ) //se esiste una stanza con questo nome
	{
		string PasswordServer = ListaStanze->DaiPasswordStanza(Messaggio);
		
		//L'utente non riesce pi� a mandare i messaggi ( va in timed out) scroprire il perch� -> il server funziona
		
		if(PasswordServer != "_")	//se � stata trovata la stanza senza alcun errore
		{	
			send(Utente,"[SERVER] Inserire la password",BUFSIZE,0);
			
			do	//inizia ad aspettare che l'utente ti invii un messaggio
			{
				while(recv(Utente,Messaggio,BUFSIZE,0)<0);	//se arriva un messaggio
				NodoUtente->UltimoMessaggio = clock();		//indipendentemente aggiorna il suo ultimo messaggio
				
			}
			while(strcmp(Messaggio,"ALIVE") == 0);	//se era un messaggio di controllo aspettane un'altro
			
			if(strcmp(Messaggio,PasswordServer.c_str()) == 0)	//se le password combaciano
			{
				send(Utente,"[SERVER] Password accettata",BUFSIZE,0);
				
				if( ListaStanze->AggiungiUtente( NomeStanza, NodoUtente) )
				{
					send(Utente,"[SERVER] Login effettuato con successo.",BUFSIZE,0);
					
					NODO *Pt1,*Pt2;
					
					for(Pt1 = ListaLobby, Pt2 = Pt1->Next; Pt2;Pt1 = Pt1->Next, Pt2 = Pt2->Next)
					{
						if(Pt2->Socket == Utente)	//se i socket combaciano
							Pt1->Next = Pt2->Next;  //rimuovilo dalla lobby
					}
			
					for(Pt1 = ListaLobby->Next; Pt1; Pt1 = Pt1->Next)		//Scorri tutta la lista degli utenti connessi alla lobby
						ListaStanze->DaiListaStanze(Pt1->Socket);			//manda la lista aggiornata delle stanze nella lobby
				}
				else
					send(Utente,"[SERVER] Impossibile effettuare il login.\n[SERVER] Per favore inserire il nome di un'altra stanza : '",BUFSIZE,0);
			}
			else
				send(Utente,"[SERVER] Password non accettata\n[SERVER] Per favore inserire il nome di un'altra stanza : ",BUFSIZE,0);
		}
		else
			send(Utente,"[SERVER] C'e' stato un problema nel trovare la stanza richiesta . . .",BUFSIZE,0);
	}
	else	//se non esiste alcuna stanza con questo nome
	{
		cout<<"Stanza non esistente . . . "<<endl;
		send(Utente,"[SERVER] Nessuna stanza con questo nome.\n[SERVER] Inserire la password per crearla : ",BUFSIZE,0);
		
		do	//inizia ad aspettare che l'utente ti invii un messaggio
		{
			while(recv(Utente,Messaggio,BUFSIZE,0)<0);	//se arriva un messaggio
			NodoUtente->UltimoMessaggio = clock();
		}
		while(strcmp(Messaggio,"ALIVE") == 0);
		
		/*=========================================================================================
			Per aggiungere un utente a una stanza, dobbiamo direttamente passare il nodo al
			metodo, sarebbe palese per� che, essendo appena stata creata una stanza, il primo
			utente � anche l'ultimo. Prima per� di poter settare il suo successore NULL, bisogna
			"rimuoverlo" dalla lista della lobby. Per rimuoverlo in realt� non si intende cancellarlo
			dalla memoria, semplicemente spostarlo da una lista a un'altra. E' importante mantenere
			l'indirizzo del suo next intatto fino alla fine, poich� dovr� essere il next del suo
			precedente. 
		-----------------------------------------------------------------------------------------*/
		
		ListaStanze->AggiungiStanza(NomeStanza,Messaggio,NodoUtente);
		
		send(Utente,"[SERVER] Stanza creata con successo . . .",BUFSIZE,0);
		
		NODO *Pt1,*Pt2;
		
		for(Pt1 = ListaLobby, Pt2 = Pt1->Next; Pt2;Pt1 = Pt1->Next, Pt2 = Pt2->Next)
		{
			if(Pt2->Socket == Utente)	//se i socket combaciano
			{
				cout<<"Utente rimosso dalla lobby e inserito in una stanza . . ."<<endl;
				cout<<"--------------------------------------------------------"<<endl;
				
				Pt1->Next = Pt2->Next;	//rimuovilo dalla lobby
				Pt2->Next = NULL;		//nell'altra lista sar� l'ultimo
			}
		}
		
		for(Pt1 = ListaLobby->Next; Pt1; Pt1 = Pt1->Next)		//Scorri tutta la lista degli utenti connessi
			ListaStanze->DaiListaStanze(Pt1->Socket);			//manda la lista aggiornata delle stanze nella lobby
	
	}

	NodoUtente->Choosing = 0;		//l'utente non sta pi� scegliendo
	
	B->LS = NULL;
	B->Messaggio = NULL;
	B->TestaLista = NULL;
	B->NodoUtente = NULL;
	
	delete B;	//Deallochiamo B
	
	return 0;
}

DWORD WINAPI ScegliNome(LPVOID lpParameter)
{
	A *B;
	
	/*==========================================================================
		Un puntatore � un intero di 4 Byte.
		Abbiamo per� questo valore in una variabile LVOID.
		Convertiamola quindi da LPVOID a un INT,
		e da INT in un puntatore di tipo (A*).
	--------------------------------------------------------------------------*/
	B = (A*) int(lpParameter);	
	
	SOCKET Utente;
	Utente = B->S;
	GestoreStanze *ListaStanze;
	ListaStanze = B->LS;
	char Messaggio[BUFSIZE];
	NODO *ListaLobby = B->TestaLista;
	NODO *NodoUtente = B->NodoUtente;
	
	bool Valid;	//variabile che controlla se il nome � disponibile o meno
	
	cout<<"Un utente sta scegliendo il nome . . ."<<endl;
	cout<<"--------------------------------------------------------"<<endl;
	
	do
	{
		cout<<"Tentativo inserimento nome . . ."<<endl;
		cout<<"--------------------------------------------------------"<<endl;
	
		Valid = true;	//per ora lo � 	
		
		send(Utente,"[SERVER] Inserire il nome che si desidera utilizzare . . .",BUFSIZE,0);
		
		do	//inizia ad aspettare che l'utente ti invii un messaggio
		{
			while(recv(Utente,Messaggio,BUFSIZE,0)<0);	//se arriva un messaggio
			NodoUtente->UltimoMessaggio = clock();
		}
		while(strcmp(Messaggio,"ALIVE") == 0);
		
		//Controlla i nomi nella lobby
		NODO *Pt;
		
		for(Pt = ListaLobby->Next; Pt && Valid; Pt = Pt->Next)	//Scorri tutta la lista degli utenti connessi nella lobby ( ammesso che il nome inserito sia ancora valido )
		{
			if( strcmp(Pt->UserName.c_str(), Messaggio ) == 0 )	//se i nomi sono uguali
				Valid = false;	//segnalalo
		}
		
		if(Valid)	//Se il nome � ancora valido
			Valid = ListaStanze->VerificaPresenzaUserName(string(Messaggio));	//allora controlla tutti i nomi nelle lobby
		
		if(!Valid)
			send(Utente,"[SERVER] Il nome inserito non e' disponibile, per favore riprovare . . .",BUFSIZE,0);
		else
			send(Utente,"[SERVER] Nome scelto con successo . . .",BUFSIZE,0);
	}
	while(!Valid);	//Riprova fino a quanto non viene inserito un username valido
	
	//Manda all'utente una lista di tutte le stanze create
	send(Utente,"[SERVER] Inserire il nome della stanza a cui si vuole connettere\n[SERVER] se la si vuole creare basta inserire il nome di una stanza inesistete.",BUFSIZE,0);
	send(Utente,"[SERVER] Lista delle stanze : ",BUFSIZE,0);
	ListaStanze->DaiListaStanze(Utente);
	
	NodoUtente->UserName = string(Messaggio);
	
	cout<<"Nome inserito con successo . . ."<<endl;
	cout<<"Nuovo nome user aggiunto : "<<NodoUtente->UserName<<endl;
	cout<<"--------------------------------------------------------"<<endl;
	
	NodoUtente->Choosing = 0;	//l'utente deve scegliere una stanza
	
	B->LS = NULL;
	B->Messaggio = NULL;
	B->TestaLista = NULL;
	B->NodoUtente = NULL;
	
	delete B;
}
