#include <string>
#include <fstream>
#include <vector>
#include <sstream> // std::stringstream
#include <iostream>
#include <limits.h>
#include <cstdlib>
#include<queue>  //queue e stack sono due librerie che permettono, rispettivamente, l'implementazione
#include <stack> //di una coda con priorità e una coda semplice di tipo last In First Out. Saranno entrambi fondamentali nell'algoritmo
#include <algorithm> //servirà solo per la funzione min
#include <regex>

using namespace std;

// Massimo numero di nodi per inizializzare l'array
constexpr int MAXN = 27560;

// Tempo soglia per cambiare mezzo inizializzato a 2 minuti
constexpr int change_time = 120;

// Questa variabile, calcolata a partire dai dati reali (vedi function read_routes)
// rappresenta le 00:00:00 del giorno di riferimento
static int ref_Time;

/*
	Definizione delle strutture dati globali
*/

// La singola Route (tratta)
struct Route
{
	int from_stop_I;
	int to_stop_I;
	int dep_time_ut;
	int arr_time_ut;
	int route_type;
	int trip_I;
	int seq;
	int route_I;
	int time;
};

// Un vertice del grafo (nodo/stazione)
struct Vertice
{
	string name;
	vector<Route> Routes; //R.ADJ
};

// Il grafo
Vertice Grafo[MAXN];

// Info Nodo: dice se il nodo è stato raggiunto e a che ora
// La Route Previous indica la tratta per raggiungerlo nel percorso minimo
struct InfoNodo
{
	bool raggiunto = false;
	int arr_time = INT_MAX;
	Route Previous;
};

// Array con le info sul raggiungimento di ogni nodo
InfoNodo* InfoNodi = new InfoNodo[MAXN];

/*
	LETTURA FILE CSV
*/

// Separatore colonne nel file csv
const char delim = ';';

// Leggi i Nodi nei Vertici del grafo
void read_nodes(string filename)
{
	// Inizializza tutti gli elementi
	for (int i = 0; i < MAXN; i++) {
		Grafo[i].name = "";
	}

	cout << endl;
	cout << "Leggo i nomi dei vertici dal file " << filename << endl;

	// Input File Stream. Per operare su file
	ifstream f(filename);

	// Verifica se il file è stato aperto correttamente
	if (!f.is_open()) {
		cout << "Impossibile aprire il file dei Nodi";
		exit(1);
	}
		

	// Per leggere una intera riga del csv
	string line;
	int stop_I;
	/*
	Alla fine queste due variabili double non sono
	servite ai fini del programma. Tuttavia le ho mantenute
	a causa del ridotto costo in tempo e in memoria che 
	provocano. Potrebbero servire in un'eventuale (e futura)
	modifica del file
	*/
	double c_lat, c_long;
	string name;
	string tmp;



	// Leggi la riga di intestazione (per saltarla)
	getline(f, line);
	int count = 0;

	// Leggi le righe con i dati, una alla vola
	while (getline(f, line))
	{
		// Crea una stringstream per la riga corrente del file
		stringstream ss(line);

		// Estrai id della stazione
		getline(ss, tmp, delim);
		stop_I = stoi(tmp);

		// Latitudine e Longitudine
		getline(ss, tmp, delim);
		c_lat = stod(tmp);
		getline(ss, tmp, delim);
		c_long = stod(tmp);

		// Nome della stazione
		getline(ss, name, delim);

		// Crea e inizializza il nodo (vertice del grafo)
		Vertice v;
		v.name = name;

		// Per adesso il vector non è inizializzato
		// Questo in c++ non provoca nessun errore
		Grafo[stop_I] = v;
		count++;
	}

	// Chiudi il file
	f.close();

	cout << "File letto correttamente. Il file " << filename 
		<< " contiene " << count << " righe." << endl;
	return;
}


// Metodo che legge network_temporal_day.csv in Routes
void read_routes(string filename) {

	cout << endl;
	cout << "Leggo le rotte da ciascun vertice dal file " << filename << endl;

	ifstream f(filename);

	// Verifica se il file è stato aperto correttamente
	if (!f.is_open()) {
		cout << "Impossibile aprire il file delle Routes";
		exit(1);
	}

	// Per leggere una intera riga del csv
	string line, tmp;
	

	// Leggi la riga di intestazione (per saltarla)
	getline(f, line);
	int count = 0;
	int min_Time = INT_MAX;


	// Leggi le righe con i dati, una alla vola
	while (getline(f, line))
	{
		// Crea una stringstream per la riga corrente del file
		stringstream ss(line);
		Route r;

		// Estrai le colonne. Anche qui ho letto tutti i campi
		// Molti si sono rivelati inutili per il programma
		getline(ss, tmp, delim);
		r.from_stop_I = stoi(tmp);

		getline(ss, tmp, delim);
		r.to_stop_I = stoi(tmp);

		getline(ss, tmp, delim);
		r.dep_time_ut = stoi(tmp);

		getline(ss, tmp, delim);
		r.arr_time_ut = stoi(tmp);

		getline(ss, tmp, delim);
		r.route_type = stoi(tmp);

		getline(ss, tmp, delim);
		r.trip_I = stoi(tmp);

		getline(ss, tmp, delim);
		r.seq = stoi(tmp);

		getline(ss, tmp, delim);
		r.route_I = stoi(tmp);

		r.time = r.arr_time_ut - r.dep_time_ut;

		if (r.from_stop_I >= MAXN)
		{
			cout << "Errore: from_stop_I troppo alto: " << r.from_stop_I;
			exit(1);
		}

		if (r.dep_time_ut < min_Time)
			min_Time = r.dep_time_ut;


		Grafo[r.from_stop_I].Routes.push_back(r);
		count++;
	}

/*
	Questo intero ci indica il tempo da sottrarre
	nella funzione from_Unix_to_Standard.
	Lo scopo è quello di rendere il più 'dinamico' possibile
	il programma, cioè fare in modo che funzioni anche
	in un ipotetico caso in cui si dovrebbe cambiare file
	e i mezzi non si riferiscano più al 12/12/2016
*/
	ref_Time = min_Time - (min_Time % 86400);

	// Chiudi il file
	f.close();

	cout << "File letto correttamente. Il file " << filename 
		<< " contiene " << count << " righe." << endl;
}


// Metodo che legge network_walk.csv in Walks
void read_walks(string filename)
{
	cout << endl;
	cout << "Leggo il file " << filename << endl;
	ifstream f(filename);
	// Verifica se il file è stato aperto correttamente
	if (!f.is_open()) {
		cout << "Impossibile aprire il file delle Walks";
		exit(1);
	}

	// Per leggere una intera riga del csv
	string line, tmp;
	// i campi di una singola riga
	int d, d_walk;


	// Leggi la riga di intestazione (per saltarla)
	getline(f, line);
	int count = 0;

	// Leggi le righe con i dati, una alla vola
	while (getline(f, line))
	{
		// Crea una stringstream per la riga corrente del file
		stringstream ss(line);
		Route r;

		// Estrai le colonne
		getline(ss, tmp, delim);
		r.from_stop_I = stoi(tmp);

		getline(ss, tmp, delim);
		r.to_stop_I = stoi(tmp);

		getline(ss, tmp, delim);
		d = stoi(tmp);

		getline(ss, tmp, delim);
		d_walk = stoi(tmp);

		r.time = d_walk / 3.889; // pre-calcolo a 5km/ora

		r.dep_time_ut = -1;
		r.arr_time_ut = -1;
		r.route_I = -1;
		r.route_type = -1;
		r.trip_I = -1;
		r.seq = -1;

		if (r.from_stop_I >= MAXN)
		{
			cout << "Errore: from_stop_I troppo alto: " << r.from_stop_I << endl;
			exit(1);
		}

		Grafo[r.from_stop_I].Routes.push_back(r);

		// Essendo una passeggiata a piedi, 
		// aggiungo anche il percorso inverso
		int temp = r.from_stop_I;
		r.from_stop_I = r.to_stop_I;
		r.to_stop_I = temp;
		Grafo[r.from_stop_I].Routes.push_back(r);

		count++;
	}

	// Chiudi il file
	f.close();

	cout << "File letto correttamente. Il file " << filename 
		<< " contiene " << count << " righe." << endl;
	return;
}

/*
	Converte l'ora in formato Unix nel formato classico leggibile dall'utente.
	Leggendo i dati di network_nodes ho notato che sono presenti fermate 
	fino a dopo la mezzanotte del lunedi.
	Questo spiega la variabile 'day' che indica il giorno corrente
*/
string from_Unix_to_Standard(int Unix)
{
/*
	ref_Time corrisponde alle 00:00 del giorno in cui
	stiamo lavorando (in questo caso il 12/12/2016), 
	in formato Unix.
	Sottraendolo, ci riduciamo a studiare un lasso 
	temporale ristretto alla singola giornata, che è 
	ciò che ci interessa.
	ref_Time viene calcolato in Read_routes
 */
	Unix -= ref_Time;

/*
	Calcolo il giorno di riferimento (day) come:
	- quello della partenza (day = 0)
	- oppure quello successivo (day=1), se si supera la mezzanotte
*/
	int day = Unix / 86400;

	// Ora calcolo ore/minuti/secondi
	int ore = (Unix / 3600) % 24;	// Numero di ore modulo 24 (nel caso in cui siamo nel giorno seguente)
	int minuti = (Unix / 60) % 60;	// /60 mi da' il numero totale dei minuti, %60 mi da' un valore <60 sottraendo quindi le ore
	int secondi = Unix % 60;		// ciò che rimane da %60 sono secondi

	string date, hh, mm, ss;

	hh = to_string(ore); 
	if (ore < 10)
		hh = '0' + hh;

	mm = to_string(minuti);
	if (minuti < 10)
		mm = '0' + mm;

	ss = to_string(secondi);
	if (secondi < 10)
		ss = '0' + ss;

	date = hh + ":" + mm + ":" + ss;

	if (day == 0)
		return date;
	else
		return date + " (giorno seguente)";
}

/*
	Funzione inversa della precedente.
	Il suo utilizzo è indispensabile ai fini del programma poiché 
	il tempo Unix verra effettivamente utilizzato per calcolare le tratte
*/
int from_Standard_to_Unix(string data)
{
	// Formato data obbligatorio: hh:mm:ss
	// Se la data è invalida restituisci -1 (errore)
	if (!regex_match(data, regex("[0-2][0-9]:[0-5][0-9]:[0-5][0-9]"))
		|| (stoi(data.substr(0, 2)) > 23))
		return -1;

	// Ottieni ore/minuti/secondi
	int ore = stoi(data.substr(0, 2));
	int minuti = stoi(data.substr(3, 2));
	int secondi = stoi(data.substr(6, 2));

	int Unix_Time = ref_Time + 3600 * ore + 60 * minuti + secondi; 
	return Unix_Time;
}

/*
Qui sta l'algoritmo vero e proprio. La modellizzazione del problema è stata implementata
similmente all'algoritmo di Dijkstra per i cammini minimi in un arco orientato e pesato.
Data la maggiore complessità del problema, tuttavia, sono entrate in gioco più strutture e variabili,
a mio avviso necessarie per la corretta riuscita del programma. La funzione verrà commentata
passo passo più nel dettaglio. Le strutture dati utilizzate sono:

- un heap di minimo, secondo la priorità del tempo di arrivo nel nodo corrente;

- una coda LIFO, necessaria poiché a partire dal nodo di destinazione si ricorstruisce a ritroso il percorso
tramite il campo stop_I;

- un array di struct InfoNodi che ci dice se il nodo è stato visitato, l'eventuale tempo di arrivo in esso e
le informazioni della tratta precedente nel cammino minimo.

Una nota da fare è che, poiché il c++ rende praticamente impossibile la ricerca di un elemento all'interno
di un heap (per modificarne la priorità come nella decrease key dell'algoritmo di Dijkstra),
ogni volta vengono aggiunti nuovi elementi provocando spesso la presenza di copie rappresentanti
lo stesso nodo con priorità diverse all'interno della coda.
Questo aumenta leggermente la complessità del programma, che tuttavia dato il relativamente basso
numero di nodi, riesce a svolgere le operazioni in tempo comunque ragionevolissimo.
*/

void Trova_Percorso(int source, int dest, int start_time, int max_time)
{
/*
	come nell'algoritmo di Dijkstra serviva un array che ci indicava la distanza dal nodo
	sorgente (la priorità del nodo), qui ci servirà in più un campo
	'raggiunto' che ci dirà se il nodo corrente è gia stato visitato.
*/
	//Struttura dati dell'elemento della Priority_queue, in particolare min_heap, implementata con le librerie standard del c++
	struct PQ {
		int stop_I;  //codice del nodo
		int arr_time_ut; //questo campo, in particolare, sarà la priorità della nostra coda
	};

	// Elemento della coda di tipo PQ
	PQ el_coda;

	// Definizione dell'operatore di confronto usato internamente dalla priority_queue
	// per determinare la priorità di un elemento durante l'inserzione e l'estrazione
	struct cmp
	{
		bool operator()(PQ const& first, PQ const& second) {
			return first.arr_time_ut > second.arr_time_ut;
		}
	};

	// coda di priorità (cmp è la struttura che realmente ce la fa diventare un heap di minimo
	// secondo arr_time_ut)
	priority_queue <PQ, vector<PQ>, cmp> pq;

	//La tratta precedente con le relative informazioni nel cammino minimo
	Route prec;

	// Inizializza Infonodi
	// altrimenti ad ogni nuovo calcolo mi ritrovo i dati del calcolo precedente
	// Inizializzo il predecessore di tutti i nodi validi
	// con tempo di arrivo infinito (INT_MAX)
	for (int i = 0; i < MAXN; i++)
	{
		InfoNodi[i].raggiunto = false;
		InfoNodi[i].arr_time = INT_MAX;

		if (Grafo[i].name != "") {
			//inizializzazione PRECEDENTE
			prec.from_stop_I = -1;
			prec.to_stop_I = i; // Nodo corrente
			prec.dep_time_ut = -1;
			prec.arr_time_ut = INT_MAX;
			prec.route_type = -2; // -1 è a piedi, quindi -2
			prec.trip_I = -1;
			prec.seq = -1;
			prec.route_I = -1;
			prec.time = INT_MAX;

			InfoNodi[i].Previous = prec;
		}
	}

	// Il nodo di partenza va inizializzato in modo diverso
	prec.from_stop_I = source;
	prec.to_stop_I = source;
	prec.dep_time_ut = start_time;
	prec.arr_time_ut = start_time;
	prec.route_type = -2;
	prec.trip_I = -1;
	prec.seq = -1;
	prec.route_I = -1;
	prec.time = 0;

	InfoNodi[source].Previous = prec;

	//inizializzazione fondamentale 
	InfoNodi[source].arr_time = start_time;

	// Mettiamo gli elementi in coda
	for (int v = 0; v < MAXN; v++) {
		if (Grafo[v].name != "") {
			el_coda.stop_I = v;
			el_coda.arr_time_ut = InfoNodi[v].arr_time;
			pq.push(el_coda);
		}

	}

	// Scorri i vertici del grafo
	while (!pq.empty())
	{
		/*
		il ciclo while termina se non ci sono piu nodi da
		analizzare.
		*/

		el_coda = pq.top();
		pq.pop();
		int u = el_coda.stop_I;
		if (!InfoNodi[u].raggiunto) {
			/*
			Da qui si procede come nell'algoritmo di Dijkstra (con la differenza che i nodi raggiunti
			saranno segnati per rendere più efficiente il programma)
			*/

			InfoNodi[u].raggiunto = true;

			//se non ci sono percorsi disponibili per l'ora selezionata, restituiamo errore

			if (InfoNodi[u].arr_time > start_time + max_time)
				continue;

			for (auto v : Grafo[u].Routes) {

				int to_stop_I = v.to_stop_I;

				//analizziamo adesso i percorsi a piedi
				if (v.route_type == -1)
				{
					if (InfoNodi[u].arr_time + v.time <= start_time + max_time)
					{
						if (InfoNodi[u].arr_time + v.time < InfoNodi[to_stop_I].arr_time)
						{
							prec.from_stop_I = u;
							prec.to_stop_I = v.to_stop_I;
							//siamo a piedi, dunque non c'è attesa; partiamo proprio quando arriviamo alla fermata.
							prec.dep_time_ut = InfoNodi[u].arr_time;
							prec.arr_time_ut = InfoNodi[u].arr_time + v.time;
							prec.route_type = -1;
							prec.trip_I = v.trip_I;
							prec.seq = v.seq;
							prec.route_I = v.route_I;
							prec.time = v.time;

							// Aggiorno le info sulla raggiungibilità del nodo e metto la struct prec al suo posto
							InfoNodi[to_stop_I].arr_time = InfoNodi[u].arr_time + v.time;
							InfoNodi[to_stop_I].Previous = prec;

							//mettiamo il nodo v in coda (secondo la sua priorità).
							//Il nodo sarà identificato dal campo to_stop
							el_coda.stop_I = to_stop_I;
							el_coda.arr_time_ut = InfoNodi[to_stop_I].arr_time;
							pq.push(el_coda);
						}
					}
				}
				else // Rotte NON a piedi
				{
					/*
						Questo intero delta_cambio ha una funzione piuttosto sottile che però mi sono sentito di dover aggiungere.
						L'idea è che se dobbiamo cambiare tra due mezzi pubblici sarà impossibile scendere dal primo e salire sul secondo
						esattamente allo stesso orario. Per questo ho scelto di far considerare al programma solo mezzi che partono due minuti
						(o più, ovviamente) dopo l'arrivo nella fermata corrente. Questo non deve essere considerato nel caso in cui:
						- Prendiamo un mezzo dopo una tratta a piedi
						- Andiamo a piedi dopo esser scesi da un mezzo
						- Non cambiamo mezzo né linea
					*/
					int delta_cambio = 0;
					if (InfoNodi[u].Previous.route_type != -1 && (InfoNodi[u].Previous.route_type != v.route_type || InfoNodi[u].Previous.route_I != v.route_I))
						delta_cambio = change_time;

					if ((v.arr_time_ut < min((start_time + max_time), InfoNodi[to_stop_I].arr_time)) && (v.dep_time_ut >= InfoNodi[u].arr_time + delta_cambio))
					{
						InfoNodi[to_stop_I].arr_time = v.arr_time_ut;

						prec.from_stop_I = u;
						prec.to_stop_I = v.to_stop_I;
						prec.dep_time_ut = v.dep_time_ut;
						prec.arr_time_ut = v.arr_time_ut;
						prec.route_type = v.route_type;
						prec.trip_I = v.trip_I;
						prec.seq = v.seq;
						prec.route_I = v.route_I;
						prec.time = v.time;

						InfoNodi[to_stop_I].Previous = prec;

						el_coda.stop_I = to_stop_I;
						el_coda.arr_time_ut = v.arr_time_ut;
						pq.push(el_coda);
					}
				}
			}
		}
	}
	return;
} // Fine Algoritmo

// Stampa il percorso trovato
void Stampa_Percorso(int source, int dest)
{
	// Esiste un percorso?
	if (InfoNodi[dest].Previous.from_stop_I == -1 || !InfoNodi[dest].raggiunto)
	{
		cout << endl;
		cout << "Nessun percorso trovato per le stazioni e l'ora selezionati." << endl;
		cout << endl;
	}
	else
	{
		/*
			Dobbiamo stampare il percorso.
			L'accorgimento di cui dobbiamo tener conto
			è che se ci sono più fermate da fare con uno stessa linea non possiamo
			stamparle tutte. Dobbiamo quindi fare un check del campo route_I e route_type
		*/
		cout << endl;
		cout << "- ECCO COSA HO TROVATO:" << endl;
		cout << endl;

		// Crea uno stack con il percorso
		stack <Route> Percorso;
		Route Fermata;
		int nodo = dest;

		// Popola lo stack: Inverti dest->start
		while (nodo != source) {
			Percorso.push(InfoNodi[nodo].Previous);
			nodo = InfoNodi[nodo].Previous.from_stop_I;
		}

		// Variabili per salvare info su stop precedente
		int prev_type = -2;
		int prev_line = -2;
		int prev_arr = -2;
		int num_fermate = 1;

		// Costruisci il percorso
		while (!Percorso.empty())
		{
			// Fai una tratta
			Fermata = Percorso.top();
			Percorso.pop();

			// Stampa l'arrivo in una stazione di interscambio solo in questi casi:
			// - Non sei alla partenza
			// - Stai per cambiare linea o mezzo
			if (Fermata.from_stop_I != source
				&& (Fermata.route_type != prev_type || Fermata.route_I != prev_line))
			{
				cout << "Arrivi alle " << from_Unix_to_Standard(prev_arr);
				cout << "  a  " << Grafo[Fermata.from_stop_I].name << " (" << Fermata.from_stop_I << ")";
				if (prev_type >= 0)
				{
					if (num_fermate == 1)
						cout << " dopo 1 fermata.";
					else
					{
						cout << " dopo " << num_fermate << " fermate.";
						num_fermate = 1;
					}
				}
				cout << endl;
			}

			// Stampa la partenza e la linea solo in questi casi:
			// - Sei alla stazione di partenza
			// - Stai cambiando linea o mezzo
			if (Fermata.route_type != prev_type || Fermata.route_I != prev_line)
			{
				cout << "Parti  alle " << from_Unix_to_Standard(Fermata.dep_time_ut);
				cout << " da  " << Grafo[Fermata.from_stop_I].name << " (" << Fermata.from_stop_I << ")";

				switch (Fermata.route_type)
				{
				case -1:
					cout << " | A PIEDI ";
					break;
				case 0:
					cout << " | con il TRAM n. " << Fermata.route_I;
					break;
				case 1:
					cout << " | con la METROPOLITANA Linea " << Fermata.route_I;
					break;
				case 2:
					cout << " | con il TRENO n." << Fermata.route_I;
					break;
				case 3:
					cout << " | con il BUS n." << Fermata.route_I;
					break;
				default:
					cout << " | con la inea n. " << Fermata.route_I;
					break;
				} // Fine switch
				cout << endl;

			} // Fine if, che stampa la partenza

			// Stampa l'eventuale arrivo a destinazione
			if (Fermata.to_stop_I == dest)
			{
				cout << "Arrivi alle " << from_Unix_to_Standard(Fermata.arr_time_ut);
				cout << "  a  " << Grafo[Fermata.to_stop_I].name << " (" << Fermata.to_stop_I << ")";
				if (Fermata.route_type >= 0)
				{
					if (num_fermate == 1)
						cout << " dopo 1 fermata.";
					else
						cout << " dopo " << num_fermate << " fermate.";
				}
				cout << endl;
			}

			// salva info stazione per il prossimo step
			if (Fermata.route_type == prev_type && Fermata.route_I == prev_line)
				num_fermate++;
			prev_type = Fermata.route_type;
			prev_line = Fermata.route_I;
			prev_arr = Fermata.arr_time_ut;
		} // Fine while (percorso non completato)
	} // Fine else (esiste un percorso)
	cout << endl;
}

/*
	Ottieni un identificativo numerico di stazione dall'input
*/
int Get_Stop_I()
{
	string strNode;
	int intNode;

	while (true)
	{
		cin >> strNode;
		if (regex_match(strNode, regex("[0-9]+")))
		{
			intNode = stoi(strNode);
			if (intNode == 0 || intNode >= 27560 || (Grafo[intNode].name == ""))
				cout << "Stazione non valida, riprovare: ";
			else
				break; // Identificativo valido. Procedi
		}
		else
			cout << "Stazione non valida. Digita l'identificativo numerico: ";
	} //Fine While

	cout << "Hai selezionato la seguente stazione: " << Grafo[intNode].name << endl;
	cout << endl;
	return intNode;
}

/*
	Ottieni in input un orario valido in formato hh:mm:ss
	(orario di partenza e durata massima del viaggio)
*/
string Get_Time_Input()
{
	string strTime;

	// Ripeti finché ottieni un oraio valido
	while (true)
	{
		cin >> strTime;
		if (regex_match(strTime, regex("[0-2][0-9]:[0-5][0-9]:[0-5][0-9]"))
				&& (stoi(strTime.substr(0, 2)) < 24))
			break; // OK, procedi
		else
		{
			cout << "Formato orario errato. Deve essere hh:mm:ss " << endl;
			cout << "Ritenta: ";
		}
	}

	cout << endl;
	return strTime;
}

/*
	Main
*/
int main()
{
	int SourceNode;
	int DestNode;
	string MaxTime;
	string StartTime;
	string strNode;

	read_nodes("network_nodes.csv");
	read_walks("network_walk.csv");
	read_routes("network_temporal_day.csv");

	cout << endl;
	
	/* Scelta opzioni e calcolo percorso fino al break. 
		Ho deciso di impostare le condizione di ciclo sempre vere
		e interroperle nel caso con il comando break perché a mio
		avviso la massiccia presenza di cicli e, di conseguenza, di variabili
		booleane danneggiava molto la leggiblità del codice. In questo modo 
		dovrebbe seere più 'snello'
	*/
	while (true)
	{
		// Ottieni SourceNode
		cout << "Inserisci il codice della stazione di partenza: ";
		SourceNode = Get_Stop_I();

		// Ottieni un DestNode != SourceNode
		while(true)
		{
			cout << "Inserisci il codice della stazione di arrivo: ";
			DestNode = Get_Stop_I();
			if (SourceNode == DestNode)
				cout << "Inserisci una destinazione diversa dalla stazione di partenza: ";
			else
				break; // OK, procedi al prossimo passaggio
		}		

		// Oraio di partenza
		cout << "Inserisci l'orario di partenza (hh:mm:ss): ";
		StartTime = Get_Time_Input();

		// Ttempo max valido
		cout << "Inserisci il tempo massimo di percorrenza (hh:mm:ss): ";
		MaxTime = Get_Time_Input();

		int start_time = from_Standard_to_Unix(StartTime);
		int max_time = from_Standard_to_Unix(MaxTime) - from_Standard_to_Unix("00:00:00");

		Trova_Percorso(SourceNode, DestNode, start_time, max_time);
		Stampa_Percorso(SourceNode, DestNode);

		cout << endl;
		string continua;
		// Ripeti fino a una scelta valida (continua o no)
		while (true)
		{
			cout << "Vuoi cercare un nuovo percorso? (Premi S per Si oppure N per No) ";
			cin >> continua;
			cout << endl;
			if (continua != "S" && continua != "s"
				&& continua != "N" && continua != "n")
				cout << "Scelta non valida. Digita un solo carattere: S oppure N." << endl;
			else
				break; // OK, scelta valida, procedi
		}
		
		if (continua == "N" || continua == "n")
			break; // fine programma, se no continua
	}

	cout << "Terminazione programma." << endl;	

	return 0;
}