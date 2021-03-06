# ProgettoChat

###### Premessa: ######
Nel scrivere il codice del server e del client inizialmente mi ero prefissato di poter trasmettere con write e read non una singola stringa bensì un'intera struttura. Tuttavia su internet esperti sconsigliavano ciò proprio per l'impossibilità di prevedere la configurazione hw con cui viene avviato il client, visto che hw diversi potrebbero ricostruire i pacchetti in entrata in maniera differente e generare errori.
Premesso ciò per evitare di modificare ulteriormente il codice ho preferito lavorare con i puntatori alla struttura. La struttura mi serve principalmente per organizzare i dati da spedire successivamente al server.
Per evitare problemi riguardanti la dimensione effettiva delle stringhe ho inserito per ogni stringa una dimensione massima di 1024, ciò per rendere portabile il codice. In futuro è possibile modificare tale dimensione senza problemi stando attenti però a particolari righe di codice.
Per evitare sovrapposizioni di output ho preferito oltre al client scrivere una sorta di writer, in modo tale da avere indipendenza tra i vari messaggi.
Client e server vanno letti in contemporanea per capire l'intero funzionamento. Soprattutto per i comandi speciali.

#### Registrazione: ####
I problemi riscontrati sono molteplici, inizialmente avevo pensato di poter spedire "in live" il nome e la password dell'utente che usa il client, tuttavia nell'eventualità in cui si instauri la connessione però l'utente per qualche motivo o per capriccio non scrive o scrive parzialmente le credenziali, il server rimaneva in attesa di tali informazioni, impiegando così risorse e bloccando la possibilità ad altri client di poter usufruire del servizio (in questo caso la registrazione).
Ho risolto il problema facendo in modo che il nome utente e la password vengano inseriti dall'utente prima di richiedere la connessione. Una volta effettuata la connessione il server controlla i dati ricevuti, se il nome utente inserito non è presente in un file nome_utenti.txt allora il server inserisce tale nome e password sequenzialmente nel file e da pieno accesso al client nella stanza Pubblica.
Attraverso un abile meccanismo se il file nome_utenti.txt non esiste e viene creato il primo utente che viene inserito è ADMIN con password ROOT, tale utente potrà utilizzare il comando /broadcast.

#### Login: ####
Come per la registrazione per evitare che il client per un tempo indeterminato bloccasse il server ho pensato di far reperire i dati dell'utente prima della connessione, in modo tale che, al momento dell'effettiva connessione, il server rilasci subito le risorse e si può dedicare ad altri client.
Il server controllerà se il nome utente e la password sono presenti in un file nome_utenti.txt, se i dati sono concordi allora darà pieno accesso al client all'utilizzo della chat nella stanza Pubblica, se il nome utente non è presente o la password è errata il server farà capire al client che c'è un errore. Il client dovrà reperire nuovamente le credenziali. Ciò accade fino a quando i dati non sono concordi.

#### Comandi: ####
###### /crea_stanza: ######
Ho inserito la possibilità di creare stanze pubbliche e private a secondo dell'utilizzo dell'utente.
Ogni messaggio inserito dall'utente viene frammentato in 5 argomenti, questo meccanismo viene utilizzato per la gestione dei comandi speciali, se infatti l'argomento 0 è appunto /crea_stanza allora il server potrà capire quale azione adoperare. Nel caso in cui l'argomento 1 fosse vuoto allora non succede assolutamente nulla. Se l'argomento 1 è non vuoto e l'argomento 2 è vuoto allora si potrebbe creare una stanza pubblica.
Ovviamente il server controllerà il nome della stanza, se è già presente in un file chiamato stanze.txt allora invierà un messaggio al client dicendo che è già presente. Se la stanza non è presente nel file verrà aggiunta. Con un meccanismo abile ho fatto in modo che se il file non esiste e viene creato come prima stanza inserita vi è proprio "Pubblica" con nessuna password presente. In modo tale che se un utente si trova ad esempio in room1 e vuole tornare in Pubblica può farlo tranquillamente.
In questo caso non abbiamo pericolo di trattenere il server per un tempo indeterminato proprio perchè gli argomenti vengono consegnati in blocco.

###### /sposta: ######
Come nel caso di /crea_stanza per /sposta bisogna inserire il nome e la password. Viene spostato solo se il nome e la password coincidono con quelli presenti nel file. Altrimenti il server restituisce l'errore al client.
Sarà sempre possibile attraverso il comando /stanza_attuale capire in quale stanza ci si trova.
Il comando /help fornisce la lista dei comandi.

###### /broadcast: ######
Solo l'utente ADMIN può utilizzare questo comando. Invia un messaggio a tutti i client in qualsiasi stanza siano.

Ho ipotizzato che il numero massimo di clienti fosse appunto 5,
nel momento in cui grazie alla select vi è un nuovo client, il descrittore viene inserito in un vettore di stringhe client_sockets[], la cosa che mi ha dato più soddisfazione per la realizzazione di questo progetto è stata la possibilità di poter sincronizzare tutte le informazioni con questi vettori. Infatti la posizione del vettore sta ad indicare univocamente quel descrittore. Quindi se vi sarà ad esempio il primo client in client_sockets[0], i rispettivi vettori client_sockets_room[], client_sockets_name[] alla medesima posizione 0 avranno le informazioni di quel determinato descrittore. E questo ha permesso di poter configurare il tutto. Nel momento in cui si collega un client, viene inserito in una posizione libera di client_sockets[], successivamente il nome di questo utente viene inserito nella stessa posizioni di un altro vettore chiamato client_sockets_name, inizialmente alla medesima posizione di client_sockets_room[] viene copiata la stringa "Pubblica", inizialmente tutti i client si collegano in "Pubblica".
Se l'utente abbandona la chat le rispettive posizione di quei vettori vengono ripulite.
