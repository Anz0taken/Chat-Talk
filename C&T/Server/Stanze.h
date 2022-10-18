#define SFIDATO 			10
#define ATTESASFIDA	 	 	9
#define INSFIDA_SFIDANTE	11
#define INSFIDA_SFIDATO		12
#define LIBERO			 	0

//ho dobuto creare la funzione poichè non mi riconosce le funzioni della libreria "string"
string to_string(int Number)
{
	string Answer;
	
	for(int i = 1; Number; i*=10)
	{
		Answer.push_back( Number%10 + 48 );
		Number /= 10;
	}
	
	int N = Answer.size();
	
	for(int i = 0; i<N; i++)
	{
		char S;
		S = Answer[i];
		Answer[i] = Answer[N - i - 1];
		Answer[N - i - 1] = S;
	}
	
	return Answer;
}

class GestoreStanze{

private:
	NODO_Stanza *Head;

public:
	GestoreStanze()
	{
		Head = new NODO_Stanza;	//creiamo il primo nodo (che non rappresenterà una stanza)
		Head->Next = NULL;		//non c'è alcun nodo
	}
	
	//chiude tutti i socket e rilascia tutta la memoria di ogni stanza e di ogni user al suo interno
	~GestoreStanze()
	{
		NODO_Stanza *Pt1_Stanza,*Pt2_Stanza;		//puntatori che scorrono ogni stanza creata
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza;)	//scorry ogni stanza di utenti
		{
			Pt2_Stanza = Pt1_Stanza->Next;			//il secondo puntatatore punta alla stanza successiva
			
			NODO *Pt1_Utenti,*Pt2_Utenti;			//creiamo in due puntatori che scorrono ogni utente di ogni stanza
			
			for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi->Next; Pt1_Utenti;)	//per ogni utente connesso alla stanza
			{
				Pt2_Utenti = Pt1_Utenti->Next;		//il secondo puntatore punta al secondo utente connesso
				closesocket(Pt1_Utenti->Socket);	//viene chiuso il socket
				delete Pt1_Utenti;					//viene cancellata la memoria
				Pt1_Utenti = Pt2_Utenti;			//passiamo al prossimo user
			}
				
			delete Pt1_Stanza;			//cancelliamo il nodo della stanza	
			Pt1_Stanza = Pt2_Stanza;	//passiamo alla stanza successiva
		}
	}
	
	/*==========================================================================
		Questa funzione, datogli il nome di una stanza, returna flase se non
		l'ha trovata, true il contraio.
		La funzione richiede:
			-Nome della stanza del server che vogliamo cercare
	--------------------------------------------------------------------------*/
	bool CercaNomeStanza(string _Nome)
	{		
		bool Answer = false;		//iniziamo dicendo che non l'ha trovata
		
		NODO_Stanza *Pt1_Stanza;	//puntatori che scorrono ogni stanza creata
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza && !Answer;Pt1_Stanza = Pt1_Stanza->Next )	//scorry ogni stanza di utenti e fino a che non abbiamo un match o fino a che non finiscono le stanze
		{
			if(Pt1_Stanza->NomeStanza == _Nome)
					Answer = true;
		}

		return Answer;
	}
	
	/*==========================================================================
		Dato il nome di una delle stanze del server, la funzione provvede
		a returnare una stringa contenente la password della stanza specifica.
	--------------------------------------------------------------------------*/
	string DaiPasswordStanza(string _Nome)
	{
		string Answer;	// '_' significa che non ha ancora trovato la stanza
		
		NODO_Stanza *Pt1_Stanza;		//puntatore che scorre ogni stanza creata
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza && !Answer.size();Pt1_Stanza = Pt1_Stanza->Next )	//scorry ogni stanza di utenti e fino a che non abbiamo un match
			if(Pt1_Stanza->NomeStanza == _Nome)
				Answer = Pt1_Stanza->Password;
		
		return Answer;
	}
	
	/*==========================================================================
		Questa funzione serve a creare una nuova stanza nel server.
		La funzione richiede:
			-Nome della stanza del server che vogliamo creare
			-Password della stanza del server che vogliamo creare
			-Il socket dell'utente che l'ha creata (in questo modo viene aggiunto direttamente)
	--------------------------------------------------------------------------*/
	void AggiungiStanza(string NomeStanza, string Password,NODO *Utente)
	{
		NODO_Stanza *Pt1_Stanza;		//puntatori che scorrono ogni stanza creata
		
		for(Pt1_Stanza = Head; Pt1_Stanza->Next; Pt1_Stanza = Pt1_Stanza->Next ); //scorri fino alla penultima stanza
		
		cout<<"Nuova stanza creata . . ."<<endl;
		cout<<"Nome : "<<NomeStanza<<endl;
		cout<<"Password : "<<Password<<endl;
		cout<<"--------------------------------------------------------"<<endl;
	
		Pt1_Stanza->Next = new NODO_Stanza;							//Creiamo una stanza
		Pt1_Stanza = Pt1_Stanza->Next;								//Puntiamola
		Pt1_Stanza->NomeStanza = NomeStanza;						//Mettici il nome
		Pt1_Stanza->Password = Password;							//La password
		
		//Ricordiamoci che il primo NON rappresenta un utente
		Pt1_Stanza->UtentiConenssi = new NODO;						//Crea il primo nodo "inutile"
		Pt1_Stanza->UtentiConenssi->Next = Utente;					//Il primo nodo sarò quello dell'utente già esistente
		Pt1_Stanza->NumeroUsers = 1;								//C'è l'utente che l'ha creata
		
		send(Utente->Socket,"[SERVER] Digitare /sfida 'NomeUtente' per sfidare un utente a tris.\n[SERVER] Una volta sfidato decidera' se accettare o meno.\n[SERVER] Se accettera' iniziera' lui per primo.",BUFSIZE,0);
		send(Utente->Socket,"[SERVER] Digitare /gioca 'DoveGiocate' per mettere il proprio segno.\n[SERVER] Digitare '/sfida esci' per uscire dalla partita.",BUFSIZE,0);
	}
	
	
	/*==========================================================================
		Questa funzione serve ad aggiungere un utente a una stanza.
		La funzione richiede:
			-Nome della stanza del server in cui l'utente vuole entrare
			-Il socket dell'utente che vuole entrare
	--------------------------------------------------------------------------*/
	bool AggiungiUtente(string NomeStanza,NODO *Utente)
	{
		NODO_Stanza *Pt1_Stanza;		//puntatori che scorrono ogni stanza creata
		bool Keep = false;
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza && !Keep; Pt1_Stanza = Pt1_Stanza->Next )	//scorri fino all'ultima stanza
		{
			if(NomeStanza == Pt1_Stanza->NomeStanza)	//se è la stanza che cerchiamo
			{
				Keep = true;		//l'abbiamo trovata
				
				NODO *Pt1_Utenti;	//Scorriamo ogni utente della stanza fino al raggiungimento dell'ultimo
				
				for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi; Pt1_Utenti->Next; Pt1_Utenti = Pt1_Utenti->Next); //scorriamo ogni utente connesso alla stanza fino all'penultimo
				
				/*=======================================================
					Utilizziamo il nodo giò esistente in memoria heap
				-------------------------------------------------------*/
				Pt1_Utenti->Next = Utente;	//l'ultimo punterà al "nuovo ultimo"
				Utente->Next = NULL;		//l'ultimo non avrò un next
				
				Pt1_Stanza->NumeroUsers++;
				
				char Messaggio[500];
				strcpy(Messaggio,"[SERVER] L'utente ");
				strcat(Messaggio,Utente->UserName.c_str());
				strcat(Messaggio," si e' appena connesso alla stanza!");
				MandaMessaggioUtentiStanzaServer(Pt1_Stanza,Utente,Messaggio);
				
				MandaNomiUtentiStanza(Pt1_Stanza,Utente->Socket);
			}
		}
		
		send(Utente->Socket,"[SERVER] Digitare /sfida 'NomeUtente' per sfidare un utente a tris.\n[SERVER] Una volta sfidato decidera' se accettare o meno.\n[SERVER] Se accettera' iniziera' lui per primo.",BUFSIZE,0);
		send(Utente->Socket,"[SERVER] Digitare /gioca 'DoveGiocate' per mettere il proprio segno.\n[SERVER] Digitare '/sfida esci' per uscire dalla partita.",BUFSIZE,0);
		
		return Keep;	//returna la risposta
	}
	
	/*==========================================================================
		Questa funzione serve a mandare la lista di tutte le stanze presenti
		nel server a un utente.
		La funzione richiede:
			-Il socket dell'utente che lo vuole sapere
	--------------------------------------------------------------------------*/
	void DaiListaStanze(SOCKET SocketNuovoUser)
	{
		NODO_Stanza *Pt1_Stanza;		//puntatore che scorre ogni stanza creata
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza; Pt1_Stanza = Pt1_Stanza->Next )	//scorri ogni stanza creata
		{
			char Msg[BUFSIZE+50];	//prepara la memoria per l'invio del messaggio
			
			strcpy(Msg,"[SERVER] ");
			
			strcat(Msg,Pt1_Stanza->NomeStanza.c_str());	//sposta il nome della stanza nell'array di char "Msg"
			
			strcat(Msg,"            [Persone : ");
			string Number;
			Number = to_string(Pt1_Stanza->NumeroUsers);
			strcat(Msg,Number.c_str());
			strcat(Msg,"]");
			
			send(SocketNuovoUser,Msg,BUFSIZE,0);		//Mandalo all'utente che l'ha richiesto
			
			/*if(send(SocketNuovoUser,Msg,BUFSIZE,0)>0)
				cout<<"Invio nome stanza : "<<Bla.c_str()<<endl;
			else
				cout<<"Errore invio N : "<<WSAGetLastError()<<endl;*/
		}
		
		send(SocketNuovoUser,"[SERVER] ------------------------------------ ",BUFSIZE,0);		//Mandalo all'utente che l'ha richiesto
	}
	
	/*==========================================================================
		Questa funzione serve a mandare un messaggio a tutti gli utenti
		di tutte le STANZE connessi al fine di potergli segnalare che il server
		è ancora funzionante.
	--------------------------------------------------------------------------*/
	void AggiornaCLientTTL()
	{
		NODO_Stanza *Pt1_Stanza;	//puntatore che scorre tutte le stanze
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza;Pt1_Stanza = Pt1_Stanza->Next)	//scorry ogni stanza di utenti
		{
			NODO *Pt1_Utenti;			//puntatore che scorre ogni utente della stanza attualle
			
			for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi->Next; Pt1_Utenti;Pt1_Utenti = Pt1_Utenti->Next)	//per ogni utente connesso alla stanza
			{
				send(Pt1_Utenti->Socket, "ALIVE", BUFSIZE, 0);	//manda il messaggio
			}
		}
	}
	
	/*==========================================================================
		Questa funzione serve a ottenere tutti i messaggi di tutti gli utenti
		connessi alle STANZE.
		In base al tipo di messaggio ricevuto, svolge diverse funzioni.
	--------------------------------------------------------------------------*/
	void RiceviMessaggiStanza()
	{
		char *Messaggio;
		Messaggio = new char[BUFSIZE];
		
		NODO_Stanza *Pt1_Stanza;	//puntatore che scorre ogni stanza
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza;Pt1_Stanza = Pt1_Stanza->Next)	//scorry ogni stanza di utenti
		{
			NODO *Pt1_Utenti;		//puntatore che scorre ogni utente nella stanza attuale
			
			for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi->Next; Pt1_Utenti;Pt1_Utenti = Pt1_Utenti->Next)	//per ogni utente connesso alla stanza
			{
				if(recv(Pt1_Utenti->Socket, Messaggio, BUFSIZE, 0)>0)	//se è arrivato un messaggio da un utente in stanza
				{						
					if(strncmp(Messaggio,"/sfida",6) == 0 )	//se il messaggio ricevuto dal client inizia con "/sfido"
					{
						if(Pt1_Utenti->Choosing == SFIDATO)	//se l'utente che ha mandato il comando è stato sfidato
						{
							int NumCharMess;	//quanti caratteri contiene il messaggio ricevuto
															
							for(NumCharMess = 0; Messaggio[NumCharMess]; NumCharMess++);	//conta quanti caratteri contiene il messaggio di sfida
							
							if(NumCharMess > 6)						//se l'utente ha inserito anche l'username che vuole sfidare
							{
								string Risposta;
								
								for(int i = 7; Messaggio[i];i++)	//incomincia a prelevare i caratteri da dopo lo spazio e mettili nella stringa "Avversario"
									Risposta.push_back(Messaggio[i]);
								
								if(Risposta == "Si")
								{
									NODO* UtenteSfida = RestituisciNodoUtente(Pt1_Utenti->Avversario,Pt1_Stanza);	//otteniamo il suo reciproco avversario
									
									if(UtenteSfida) //se è stato trovato l'utente che ha sfidato
									{
										//comunica all'utente (sfidato) che ha accettato la sfida
										send(Pt1_Utenti->Socket,"[SERVER] Hai ha accettato la sfida.\n[SERVER] Partita iniziata ufficialmente.",BUFSIZE,0);
										Pt1_Utenti->Choosing = INSFIDA_SFIDATO;
										Pt1_Utenti->Turno = true;	//è il turno dello sfidato
										
										//comunica all'utente (sfidante) che è stata accettata la sfida
										send(UtenteSfida->Socket,"[SERVER] Il tuo avversario ha accettato la sfida.\n[SERVER] Partita iniziata ufficialmente.",BUFSIZE,0);
										UtenteSfida->Choosing = INSFIDA_SFIDANTE;
										UtenteSfida->PartitaTris = new Tris;
										UtenteSfida->Turno = false;	//non è il turno dello sfidante
										
										Pt1_Utenti->PartitaTris = UtenteSfida->PartitaTris;	//anche lo sfidato punta allo stesso oggetto tris
										
										string Rigo_1;
										Rigo_1.push_back('0');
										Rigo_1.push_back('|');
										Rigo_1.push_back('1');
										Rigo_1.push_back('|');
										Rigo_1.push_back('2');
										Rigo_1.push_back(0);
										string Rigo_2;
										Rigo_2.push_back('3');
										Rigo_2.push_back('|');
										Rigo_2.push_back('4');
										Rigo_2.push_back('|');
										Rigo_2.push_back('5');
										Rigo_2.push_back(0);
										string Rigo_3;
										Rigo_3.push_back('6');
										Rigo_3.push_back('|');
										Rigo_3.push_back('7');
										Rigo_3.push_back('|');
										Rigo_3.push_back('8');
										Rigo_3.push_back(0);
										
										send(Pt1_Utenti->Socket,"[SERVER] Mostro tris . . .",BUFSIZE,0);
										send(Pt1_Utenti->Socket,Rigo_1.c_str(),BUFSIZE,0);
										send(Pt1_Utenti->Socket,"+++++",BUFSIZE,0);
										send(Pt1_Utenti->Socket,Rigo_2.c_str(),BUFSIZE,0);
										send(Pt1_Utenti->Socket,"+++++",BUFSIZE,0);
										send(Pt1_Utenti->Socket,Rigo_3.c_str(),BUFSIZE,0);
										
										send(UtenteSfida->Socket,"[SERVER] Mostro tris . . .",BUFSIZE,0);
										send(UtenteSfida->Socket,Rigo_1.c_str(),BUFSIZE,0);
										send(UtenteSfida->Socket,"+++++",BUFSIZE,0);
										send(UtenteSfida->Socket,Rigo_2.c_str(),BUFSIZE,0);
										send(UtenteSfida->Socket,"+++++",BUFSIZE,0);
										send(UtenteSfida->Socket,Rigo_3.c_str(),BUFSIZE,0);
									}
									else	//se l'utente non è stato trovato, potrebbe essere uscito dal server o chrashato
									{
										//pertanto il nostro utente non sarò più in alcuna sfida
										Pt1_Utenti->Choosing = LIBERO;
										Pt1_Utenti->Avversario = "";
										send(Pt1_Utenti->Socket,"[SERVER] L'utente a cui hai accettato la richiesta non e' piu' online.\n[SERVER] Pertanto non sei piu' in una sfida.",BUFSIZE,0);
									}
								}
								else if(Risposta == "No")
								{
									NODO* UtenteSfida = RestituisciNodoUtente(Pt1_Utenti->Avversario,Pt1_Stanza);	//otteniamo il suo reciproco avversario
									
									if(UtenteSfida) //se è stato trovato l'utente da sfidare
									{
										send(UtenteSfida->Socket,"[SERVER] Il tuo avversario non ha accettato la sfida",BUFSIZE,0);	
										UtenteSfida->Choosing = LIBERO;
										UtenteSfida->Avversario = "";						
									}
									
									Pt1_Utenti->Choosing = LIBERO;
									Pt1_Utenti->Avversario = "";
								}
							}
						}
						else if(Pt1_Utenti->Choosing == ATTESASFIDA || Pt1_Utenti->Choosing == INSFIDA_SFIDANTE || Pt1_Utenti->Choosing == INSFIDA_SFIDATO)	//se l'utente che ha mandato il comando è in attesa di una risposta o è già entrato nella sfida
						{
							int NumCharMess;	//quanti caratteri contiene il messaggio ricevuto
															
							for(NumCharMess = 0; Messaggio[NumCharMess]; NumCharMess++);	//conta quanti caratteri contiene il messaggio di sfida
							
							if(NumCharMess > 6)						//se l'utente ha inserito anche l'username che vuole sfidare
							{
								string Risposta;
								
								for(int i = 7; Messaggio[i];i++)	//incomincia a prelevare i caratteri da dopo lo spazio e mettili nella stringa "Avversario"
									Risposta.push_back(Messaggio[i]);
									
								if(Risposta == "esci")	//Se l'utente vuole uscire dalla sfida
								{
									NODO* UtenteSfida = RestituisciNodoUtente(Pt1_Utenti->Avversario,Pt1_Stanza);	//otteniamo il suo reciproco avversario
									
									if(UtenteSfida) //se è stato trovato l'utente da sfidare
									{
										send(UtenteSfida->Socket,"[SERVER] Il tuo avversario ha annullato la sfida.",BUFSIZE,0);	
										UtenteSfida->Choosing = LIBERO;
										UtenteSfida->Avversario = "";

										if(UtenteSfida->PartitaTris)	//se era la persona che stava hostando la partita
										{
											delete UtenteSfida->PartitaTris;
											UtenteSfida->PartitaTris = NULL;
											Pt1_Utenti->PartitaTris = NULL;
										}
									}
									
									Pt1_Utenti->Choosing = LIBERO;
									Pt1_Utenti->Avversario = "";
									if(Pt1_Utenti->PartitaTris)	//se era la persona che stava hostando la partita
									{
										delete Pt1_Utenti->PartitaTris;
										Pt1_Utenti->PartitaTris = NULL;
									}
									
									send(Pt1_Utenti->Socket,"[SERVER] Sfida annullata correttamente.",BUFSIZE,0);
								}
							}
						}
						else	//se l'utente non è gia stato sfidato o non ha mandato una sfida, allora vuole sfidare ora un utente
						{
							cout<<"L'utente "<<Pt1_Utenti->UserName<<" vuole avviare una sfida . . ."<<endl;
							cout<<"--------------------------------------------------------"<<endl;
							
							if(Pt1_Utenti->Avversario.size() == 0)	//se l'utente che ha mandato una sfida non è già in una sfida e non è in attesa di risposta di alcuna sfida
							{
								cout<<"L'utente "<<Pt1_Utenti->UserName<<" non e' gia' una sfida . . ."<<endl;
								cout<<"--------------------------------------------------------"<<endl;
								/*==========================================================================
									Sappiamo quindi che ora,
									l'utente che ha inviato la sfida non è in alcuna sfida.
									Bisogna ora però assicurarci che anche l'utente che è stato sfidato
									non è in alcuna sfida.
									
									Per fare questo, dobbiamo ottenere il nome dell'utente che vuole sfidare.
									
									01234567........
									/sfido xxxxxxxxx 
										   ^	   ^
										   I	   F
										   N	   I
										   I	   N
										   Z	   E
										   I
										   O
										   
									Otteniamo quindi il nome . . .
									
									Prima di ottenere il nome, dobbiamo prima assicurarci che l'utente non
									abbia inserito solo /sfido.
								--------------------------------------------------------------------------*/
								int NumCharMess;	//quanti caratteri contiene il messaggio ricevuto
																
								for(NumCharMess = 0; Messaggio[NumCharMess]; NumCharMess++);	//conta quanti caratteri contiene il messaggio di sfida
								
								if(NumCharMess > 6)						//se l'utente ha inserito anche l'username che vuole sfidare
								{
									string Avversario;
									
									for(int i = 7; Messaggio[i];i++)	//incomincia a prelevare i caratteri da dopo lo spazio e mettili nella stringa "Avversario"
										Avversario.push_back(Messaggio[i]);
									
									if(Pt1_Utenti->UserName != Avversario)	//se l'utente non ha sfidato se stesso
									{
										NODO* UtenteSfidato = RestituisciNodoUtente(Avversario,Pt1_Stanza);
										if(UtenteSfidato)	//se esiste un'utente con quel nickname, quindi esiste il nodo
										{
											if(UtenteSfidato->Choosing != ATTESASFIDA && UtenteSfidato->Choosing != SFIDATO)	//Se l'utente non è gia stato sfidato e non è in attesa
											{
												UtenteSfidato->Choosing = SFIDATO;	//l'utente sfidato è ora in una stato di "sfidato"
												UtenteSfidato->Avversario = Pt1_Utenti->UserName;	//diamo l'username dell'utente sfidante dallo sfidato
												
												Pt1_Utenti->Choosing = ATTESASFIDA;	//l'utente che l'ha sfidato è in attesa
												Pt1_Utenti->Avversario = UtenteSfidato->UserName;	//diamo l'username dell'utente sfidato dallo sfidante
												
												cout<<"E' stato sfidato l'utente : "<<Avversario<<" | Da : "<<Pt1_Utenti->UserName<<endl;
												cout<<"--------------------------------------------------------"<<endl;
												send(Pt1_Utenti->Socket,"[SERVER] E' stata mandata la richeista di sfida all'utente correttamente.",BUFSIZE,0);
												send(UtenteSfidato->Socket,"[SERVER] Sembra che tu sia stato sfidato. Scrivi '/sfida Si' o '/sfida No' per accettare/rifiutare.",BUFSIZE,0);
											}
											else
												send(Pt1_Utenti->Socket,"[SERVER] Sembra che l'utente sia gia' impegnato in una sfida, riprova piu' tardi.",BUFSIZE,0);	
										}
										else
											send(Pt1_Utenti->Socket,"[SERVER] Sembra che l'utente al momento non sia connesso al server, riprova piu' tardi.",BUFSIZE,0);	
									}
									else
									{
										cout<<"L'utente "<<Pt1_Utenti->UserName<<" ha provato a sfidare se stesso."<<endl;
										cout<<"--------------------------------------------------------"<<endl;
										send(Pt1_Utenti->Socket,"[SERVER] Spiacenti, non puoi sfidare te stesso.",BUFSIZE,0);									
									}						
								}
								else
								{
									cout<<"L'utente "<<Pt1_Utenti->UserName<<" non ha inserito alcun nome da sfidare."<<endl;
									cout<<"--------------------------------------------------------"<<endl;
									send(Pt1_Utenti->Socket,"[SERVER] Spiacenti, non hai inserito il nome dell'utente da sfidare.",BUFSIZE,0);
								}
							}
							else
							{
								cout<<"L'utente "<<Pt1_Utenti->UserName<<" e' gia' in una sfida."<<endl;
								cout<<"--------------------------------------------------------"<<endl;
								send(Pt1_Utenti->Socket,"[SERVER] Spiacenti, sei gia' in una sfida con un altro utente.",BUFSIZE,0);
							}
						}
					}
					else if(strncmp(Messaggio,"/gioco",6) == 0 && Pt1_Utenti->PartitaTris) //se il messaggio ricevuto dal client inizia con "/gioco" ed è in una sfida	
					{
						int NumCharMess;	//quanti caratteri contiene il messaggio ricevuto
														
						for(NumCharMess = 0; Messaggio[NumCharMess]; NumCharMess++);	//conta quanti caratteri contiene il messaggio di sfida
						
						if(NumCharMess > 6)						//se l'utente ha inserito anche dove vuole giocare
						{
							string Risposta;
							
							for(int i = 7; Messaggio[i];i++)	//incomincia a prelevare i caratteri da dopo lo spazio e mettili nella stringa "Risposta"
								Risposta.push_back(Messaggio[i]);
							
							if(Risposta.size() == 1)	//se è stato messo un solo carattere
							{
								int Cella = Risposta.at(0) - 48;
								
								/*===================================================================
									Ogni nodo di ogni utente avrà un puntatore a oggetto di tipo
									"Tris". Quando due utenti giocano, giocano sullo stesso oggetto.
								===================================================================*/
								
								if(Cella>=0 && Cella<=8)	//se è stato inserito un dato valido
								 {
									if(Pt1_Utenti->Turno)	//Se è il turno di questo giocatore/se è in corso un gioco
									{
										if(Pt1_Utenti->PartitaTris->InserisciSegno(Cella))	//se è stato inserito il segno in una cella valida
										{
											NODO* UtenteSfida = RestituisciNodoUtente(Pt1_Utenti->Avversario,Pt1_Stanza);	//otteniamo il suo reciproco avversario
											
											string Campo = Pt1_Utenti->PartitaTris->DaiCampo();
											
											string Rigo_1;
											Rigo_1.push_back(Campo[0]);
											Rigo_1.push_back('|');
											Rigo_1.push_back(Campo[1]);
											Rigo_1.push_back('|');
											Rigo_1.push_back(Campo[2]);
											Rigo_1.push_back(0);
											string Rigo_2;
											Rigo_2.push_back(Campo[3]);
											Rigo_2.push_back('|');
											Rigo_2.push_back(Campo[4]);
											Rigo_2.push_back('|');
											Rigo_2.push_back(Campo[5]);
											Rigo_2.push_back(0);
											string Rigo_3;
											Rigo_3.push_back(Campo[6]);
											Rigo_3.push_back('|');
											Rigo_3.push_back(Campo[7]);
											Rigo_3.push_back('|');
											Rigo_3.push_back(Campo[8]);
											Rigo_3.push_back(0);
											
											send(Pt1_Utenti->Socket,"[SERVER] Mostro tris . . .",BUFSIZE,0);
											send(Pt1_Utenti->Socket,Rigo_1.c_str(),BUFSIZE,0);
											send(Pt1_Utenti->Socket,"+++++",BUFSIZE,0);
											send(Pt1_Utenti->Socket,Rigo_2.c_str(),BUFSIZE,0);
											send(Pt1_Utenti->Socket,"+++++",BUFSIZE,0);
											send(Pt1_Utenti->Socket,Rigo_3.c_str(),BUFSIZE,0);
											
											send(UtenteSfida->Socket,"[SERVER] Mostro tris . . .",BUFSIZE,0);
											send(UtenteSfida->Socket,Rigo_1.c_str(),BUFSIZE,0);
											send(UtenteSfida->Socket,"+++++",BUFSIZE,0);
											send(UtenteSfida->Socket,Rigo_2.c_str(),BUFSIZE,0);
											send(UtenteSfida->Socket,"+++++",BUFSIZE,0);
											send(UtenteSfida->Socket,Rigo_3.c_str(),BUFSIZE,0);
												
											if(Pt1_Utenti->PartitaTris->Vincitore())	//controlla se c'è un vincitore
											{
												send(Pt1_Utenti->Socket,"[SERVER] Complimenti, hai vinto la sfida.",BUFSIZE,0);
									
												if(UtenteSfida) //se è stato trovato l'utente da sfidare
												{
													send(UtenteSfida->Socket,"[SERVER] Hai perso!",BUFSIZE,0);	
													UtenteSfida->Choosing = LIBERO;
													UtenteSfida->Avversario = "";
													if(UtenteSfida->PartitaTris)	//se era la persona che stava hostando la partita
													{
														delete UtenteSfida->PartitaTris;
														UtenteSfida->PartitaTris = NULL;
														Pt1_Utenti->PartitaTris = NULL;
													}
												}
												
												Pt1_Utenti->Choosing = LIBERO;
												Pt1_Utenti->Avversario = "";
												if(Pt1_Utenti->PartitaTris)	//se era la persona che stava hostando la partita
												{
													delete Pt1_Utenti->PartitaTris;
													Pt1_Utenti->PartitaTris = NULL;
												}
											}
											else if(Pt1_Utenti->PartitaTris->ContinuaPartita())	//se ancora devono essere stati inseriti tutti gli spazi del tris
											{								
												if(UtenteSfida) //se è stato trovato l'utente da sfidare
												{
													UtenteSfida->Turno = true;
													Pt1_Utenti->Turno = false;
												}
											}
											else	//se è un pareggio
											{
												send(Pt1_Utenti->Socket,"[SERVER] Peccato, pareggio!",BUFSIZE,0);
									
												if(UtenteSfida) //se è stato trovato l'utente da sfidare
												{
													send(UtenteSfida->Socket,"[SERVER] Peccato, pareggio!",BUFSIZE,0);
													UtenteSfida->Choosing = LIBERO;
													UtenteSfida->Avversario = "";
													if(UtenteSfida->PartitaTris)	//se era la persona che stava hostando la partita
													{
														delete UtenteSfida->PartitaTris;
														UtenteSfida->PartitaTris = NULL;
														Pt1_Utenti->PartitaTris = NULL;
													}
												}
												
												Pt1_Utenti->Choosing = LIBERO;
												Pt1_Utenti->Avversario = "";
												if(Pt1_Utenti->PartitaTris)	//se era la persona che stava hostando la partita
												{
													delete Pt1_Utenti->PartitaTris;
													Pt1_Utenti->PartitaTris = NULL;
												}
											}
										}
									}
								}
							}
						}	
					}
					else if(strncmp(Messaggio,"/listautenti",6) == 0 )	//se il messaggio ricevuto dal client inizia con "/listautenti"
					{
						MandaNomiUtentiStanza(Pt1_Stanza,Pt1_Utenti->Socket);
					}
					else
						if(strcmp(Messaggio,"ALIVE")!=0)	//se non è un messaggio di controllo
							MandaMessaggioUtentiStanza(Pt1_Stanza,Pt1_Utenti,Messaggio);	//manda il messaggio a tutti gli utenti PRESENTI NELLA SUA STANZA
					
					Pt1_Utenti->UltimoMessaggio = clock();	//in ogni caso, aggiorna l'ultimo messaggio ricevuto da questo utente
				}
			}
		}
		
		delete Messaggio;
	}
	
	/*==========================================================================
		Questa funzione serve a verificare che tutti gli utenti precedentemente
		connessi al server (controlla solo quelli nelle stanze)
		lo siano ancora, nel caso contrario il socket viene
		chiuso e la memoria dedicata all'utente interessato rilasciata.
	--------------------------------------------------------------------------*/
	void UtentiAttivi()
	{
		NODO_Stanza *Pt1_Stanza;	//puntatore che scorre ogni stanza del server
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza;Pt1_Stanza = Pt1_Stanza->Next)	//scorry ogni stanza di utenti
		{
			NODO *Pt1_Utenti,*Pt2_Utenti;	//creiamo in due puntatori che scorrono ogni utente di ogni stanza
			
			for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi,Pt2_Utenti = Pt1_Utenti->Next;Pt1_Utenti && Pt2_Utenti;Pt1_Utenti = Pt1_Utenti->Next,Pt2_Utenti = Pt2_Utenti->Next)	//per ogni utente connesso alla stanza
			{
				if(clock() - Pt2_Utenti->UltimoMessaggio>TIMETOLIVE)	//se è passato troppo tempo dall'ultimo messaggio ricevuto
				{				
					//Prima di eliminare il nodo, dobbiamo diren al suo eventuale avversario che c'è stata una disconnessione
					if(Pt2_Utenti->Avversario.size())	//se l'utente che stiamo per cancellare aveva un avversario
					{
						NODO* UtenteSfidato = RestituisciNodoUtente(Pt2_Utenti->Avversario,Pt1_Stanza);
						
						if(UtenteSfidato) //se è stato trovato l'utente da sfidare
						{
							send(UtenteSfidato->Socket,"[SERVER] Il tuo avversario e' uscito dal server . . .",BUFSIZE,0);	
							UtenteSfidato->Choosing = LIBERO;
							UtenteSfidato->Avversario = "";			
						}
					}
					
					char Messaggio[500];
					strcpy(Messaggio,"[SERVER] L'utente ");
					strcat(Messaggio,Pt2_Utenti->UserName.c_str());
					strcat(Messaggio," si e' disconnesso.");
					MandaMessaggioUtentiStanzaServer(Pt1_Stanza,Pt2_Utenti,Messaggio);
					
					cout<<"Un utente si e' disconnesso da una stanza (timed out). . ."<<endl;
					cout<<"Nome utente : "<<Pt2_Utenti->UserName<<endl;
					cout<<"IP          : "<<Pt2_Utenti->IpHost<<endl;
					cout<<"--------------------------------------------------------"<<endl;
					
					Pt1_Utenti->Next = Pt2_Utenti->Next;	//il nodo precedente a esso punta al prossimo del nodo che vogliamo cancellare
					closesocket(Pt2_Utenti->Socket);		//viene chiuso il socket
					delete Pt2_Utenti;						//viene rilasciata la memoria
					
					Pt1_Stanza->NumeroUsers--;				//c'è un utente in meno
				}
			}
		}
	}
	
	/*==========================================================================
		Questa funzione serve a mandare un messaggio a tutti gli utenti
		connessi nella stessa stanza (tranne il mittente).
		La funzione richiede:
			-Il puntatore alla stanza interessata
			-Il socket dell'utente mittente
			-Il messaggio
	--------------------------------------------------------------------------*/
	void MandaMessaggioUtentiStanza(NODO_Stanza *StanzaInteressata,NODO *Mittente,char *Messaggio)
	{
		NODO *Pt1_Utenti;			//per ogni utente connesso alla stanza
		
		for(Pt1_Utenti = StanzaInteressata->UtentiConenssi->Next; Pt1_Utenti;Pt1_Utenti = Pt1_Utenti->Next)	//scorri ogni utente
		{
			if(Pt1_Utenti != Mittente)			//se lo user a cui vogliamo mandarlo non è il mittente
			{	
				char UserName[BUFSIZE+50];
				
				strcpy(UserName,Mittente->UserName.c_str());
				
				char A[4];
				A[0] = A[2] = ' ';
				A[1] = ':';
				A[3] = 0;
				
				strcat(UserName,A);
				strcat(UserName,Messaggio);
				
				send(Pt1_Utenti->Socket,UserName,BUFSIZE,0);	//mandalo
			}
		}
	}
	
	//esegue la stessa funsione della subroutine di sopra, ma senza mettere il nickname
	void MandaMessaggioUtentiStanzaServer(NODO_Stanza *StanzaInteressata,NODO *Mittente,char *Messaggio)
	{
		NODO *Pt1_Utenti;			//per ogni utente connesso alla stanza
		
		for(Pt1_Utenti = StanzaInteressata->UtentiConenssi->Next; Pt1_Utenti;Pt1_Utenti = Pt1_Utenti->Next)	//scorri ogni utente
		{
			if(Pt1_Utenti != Mittente)			//se lo user a cui vogliamo mandarlo non è il mittente
			{
				send(Pt1_Utenti->Socket,Messaggio,BUFSIZE,0);	//mandalo
			}
		}
	}
	
	void MandaNomiUtentiStanza(NODO_Stanza *StanzaInteressata,SOCKET Utente)
	{
		NODO *Pt1_Utenti;			//per ogni utente connesso alla stanza
		
		send(Utente,"[SERVER] Mando lista utenti connessi . . .",BUFSIZE,0);
		
		for(Pt1_Utenti = StanzaInteressata->UtentiConenssi->Next; Pt1_Utenti;Pt1_Utenti = Pt1_Utenti->Next)	//scorri ogni utente
		{	
			send(Utente,Pt1_Utenti->UserName.c_str(),BUFSIZE,0);	//mandalo
		}
		
		send(Utente,"------------------------------------------",BUFSIZE,0);
	}
	
	/*==========================================================================
		Questa funzione serve a cercare se è già presente un UserName specifico
		in tutte le stanze del server.
		La funzione richiede:
			-L'UserName che si vuole cercare
			
		Restituisce false se è già presente, al contrario true 			
	--------------------------------------------------------------------------*/
	bool VerificaPresenzaUserName(string _Nome)
	{
		bool Answer = true;
		NODO_Stanza *Pt1_Stanza;	//puntatore che scorre ogni stanza
		
		for(Pt1_Stanza = Head->Next; Pt1_Stanza && Answer;Pt1_Stanza = Pt1_Stanza->Next)	//scorry ogni stanza di utenti (fino a quando il nome è valido)
		{
			NODO *Pt1_Utenti;		//puntatore che scorre ogni utente nella stanza attuale
			
			for(Pt1_Utenti = Pt1_Stanza->UtentiConenssi->Next; Pt1_Utenti && Answer;Pt1_Utenti = Pt1_Utenti->Next)	//per ogni utente connesso alla stanza (fino a quando il nome è valido)
			{
				if(strcmp(Pt1_Utenti->UserName.c_str(),_Nome.c_str()) == 0)	//se i nomi di questi utenti combacia, allora non sarò più valido
				{
					Answer = false;
					cout<<"Trovato un nome uguale . . ."<<endl;
					cout<<"--------------------------------------------------------"<<endl;
				}
			}
		}			

		return Answer;
	}
	
	NODO* RestituisciNodoUtente(string _Nome, NODO_Stanza *Stanza)
	{
		NODO* Answer = NULL;
		
		NODO *Pt1_Utenti;		//puntatore che scorre ogni utente nella stanza attuale
		
		for(Pt1_Utenti = Stanza->UtentiConenssi->Next; Pt1_Utenti && !Answer;Pt1_Utenti = Pt1_Utenti->Next)	//per ogni utente connesso alla stanza (fino a quando il nome è valido)
		{
			if(strcmp(Pt1_Utenti->UserName.c_str(),_Nome.c_str()) == 0)	//se i nomi di questi utenti combacia, allora non sarò più valido
			{
				Answer = Pt1_Utenti;
				cout<<"Trovato un nome uguale, restituisco indirizzo nodo . . ."<<endl;
				cout<<"--------------------------------------------------------"<<endl;
			}
		}			

		return Answer;	
	}
};
