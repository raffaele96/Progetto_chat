#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h> 

struct my_message
{
    long int message_type;
    char messaggio[1024];
};

struct Pacchetto_server    //mi serve per tenere organizzati i campi in futuro da aggiungere da spedire a server
{
    char nome_utente[1024];
    char messaggio[1024];
    char password[1024];
    char stanza[1024];
    char password_stanza[1024];
};

int main (void)
{
int msg_id,sock,porta,i=0;
pid_t pid;
struct my_message message;
struct sockaddr_in indirizzo_server;
struct Pacchetto_server * Pacchetto;
char ip_address[15];
char Arg[5][1024];
Pacchetto=(struct Pacchetto_server*)malloc(sizeof(struct Pacchetto_server));

msg_id = msgget ((key_t) 12345, 0666 | IPC_CREAT);
if(msg_id==-1)
{
    perror("errore msg_id del writer");
    exit(EXIT_FAILURE);
}

printf("Inserisci l'ip del server: \n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
strcpy(ip_address,message.messaggio);
printf("Inserisci la porta del server: \n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
porta=atoi(message.messaggio);


indirizzo_server.sin_family=AF_INET;
indirizzo_server.sin_port=htons(porta);
indirizzo_server.sin_addr.s_addr=inet_addr(ip_address);

do //inizio blocco di codice per la registrazione e il login
{
sock=socket(AF_INET,SOCK_STREAM,0);
if(sock==-1)
{
perror("errore creazione socket");
exit(EXIT_FAILURE);
}
do //si sblocca quando la parola è "registrazione" o "login"
{
printf("Digita: 'registrazione' oppure 'login' \n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
} while(strcmp(message.messaggio,"registrazione\0")!=0 && strcmp(message.messaggio,"login")!=0);

if(strcmp(message.messaggio,"registrazione\0")==0) i=1; //il valore di i serverirà in seguito riga 122 123

strcpy(Pacchetto->messaggio,message.messaggio); //contiene la parola registrazione o login
printf("[SERVER] Inserisci nome utente: \n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
strcpy(Pacchetto->nome_utente,message.messaggio);
printf("[SERVER] Inserisci password: \n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
strcpy(Pacchetto->password,message.messaggio);
/*la fase precedente è fondamentale, poiché non tiene il server impegnato nel momento in cui
l'utente si assenta nella schermata di login. Il primo contatto con il server si verifica quando già
si avranno a disposizione i dati.*/ 
if(connect(sock,(struct sockaddr*)&indirizzo_server,sizeof(indirizzo_server))==-1)
{
perror("errore connect");
exit(EXIT_FAILURE);
}
write(sock,Pacchetto->messaggio,sizeof(Pacchetto->messaggio)); //indica al server la decisione dell'utente.
write(sock,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente)); //invia nome utente
write(sock, Pacchetto->password,sizeof(Pacchetto->password)); //invia password;
read(sock,Pacchetto->messaggio,sizeof(Pacchetto->messaggio)); //esito operazione 
if(strcmp(Pacchetto->messaggio,"ok\0")!=0) 
{
    close(sock);
    if(i==1) printf("[SERVER] ERRORE: nome utente esistente. Prova ad effettuare il login.\n");
    if(i==0) printf("[SERVER] Nome utente non esistente o password errata.\n");
}
} while(strcmp(Pacchetto->messaggio,"ok\0")!=0 ||strcmp(Pacchetto->nome_utente,"[SERVER]\0")==0); //fine blocco di codice per la registrazione e login

/*Nel server è stata impostata la possibilità di poter registrarsi senza inserire la password*/


printf("Connesso al server.\n");

/*Da qui in poi un processo sarà incaricato di scrivere sul descrittore del server e un altro sarà incaricato di leggere
i messaggi provenienti dal server, così da poter permettere l'utilizzo di un writer e impedire la sovrapposizione 
degli output*/

pid=fork();
switch(pid)
{
case -1:
perror("Errore fork");
exit(EXIT_FAILURE);

case 0:
while(1)
{
for(i=0;i<5;i++) strcpy(Arg[i],""); //server ad "inizializzare" le stringhe degli argomenti
i=0;

if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
sscanf(message.messaggio,"%s %s %s %s %s",Arg[0],Arg[1],Arg[2],Arg[3],Arg[4]); //permette di gestire i comandi speciali
if(strcmp(Arg[0],"/help\0")==0)
{
    printf("Comandi:\n");
    printf("/crea_stanza [nome stanza] [password stanza]\n");
    printf("/sposta [nome stanza] [password stanza]\n");
    printf("/stanza_attuale\n");
    printf("/broadcast (funziona solo per admin)\n");
    i=1;
}

if(strcmp(message.messaggio,"/crea_stanza\0")==0 && i==0) //serve per evitare che l'utente non inserisca gli argomenti neces.
{
printf("Digitare: /crea_stanza [nome stanza] [password stanza]\n");
i=1;
}

if(strcmp(message.messaggio,"/sposta\0")==0 && i==0) //serve per evitare che l'utente non inserisca gli argomenti neces.
{
printf("Digitare: /sposta [nome stanza] [password stanza]\n");
i=1;
}

if(strcmp(Arg[0],"/crea_stanza\0")==0 && strcmp(Arg[1],"")!=0 && i==0 ) //questo nel caso in cui si creino stanze pubbliche
{
    i=1;
    strcpy(Pacchetto->stanza,Arg[1]);
    strcpy(Pacchetto->password_stanza,Arg[2]);
    if(strcmp(Pacchetto->password_stanza,"")==0) printf("Nessuna password inserita.\n");
    write(sock,Arg[0],sizeof(Arg[0])); //invia al server la decisione di creare la stanza
    write(sock,Pacchetto->stanza,sizeof(Pacchetto->stanza));
    write(sock,Pacchetto->password_stanza,sizeof(Pacchetto->password_stanza));
}

if(strcmp(Arg[0],"/sposta\0")==0 && strcmp(Arg[1],"") !=0 && i==0) //questo in cui si creino stante private
{
    i=1;
    strcpy(Pacchetto->stanza,Arg[1]);
    strcpy(Pacchetto->password_stanza,Arg[2]);
    write(sock,Arg[0],sizeof(Arg[0])); //invia al server la decisione di spostarsi in una stanza
    write(sock,Pacchetto->stanza,sizeof(Pacchetto->stanza));
    write(sock,Pacchetto->password_stanza,sizeof(Pacchetto->password_stanza));
}


if(strcmp(message.messaggio,"/stanza_attuale\0")==0 && i==0) //server per evitare errori da parte dell'utente.
{
i=1;
write(sock,message.messaggio,sizeof(message.messaggio));
}

if(strcmp(message.messaggio,"/broadcast\0")==0 && i==0) // gestione del comando /broadcast
{
i=1;
strcpy(Pacchetto->messaggio,"/broadcast\0");
printf("[SERVER] Il successivo messaggio verrà inviato a tutti i client:\n");
if(msgrcv(msg_id,(void*)&message,sizeof(message),1,0)==-1)
{
    perror("errore msgrcv client");
    exit(EXIT_FAILURE);
}
write(sock,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
write(sock,message.messaggio,sizeof(message.messaggio));
}

if(strcmp(message.messaggio,"end\0")==0 && i==0) //server per chiudere il processo figlio rilasciando le risorse
{
i=1;
write(sock,message.messaggio,sizeof(message.messaggio));
free(Pacchetto);
exit(EXIT_SUCCESS); //la coda messaggi viene rimossa dal writer
}

if(i==0) //se i==0 vuol dire che è un messaggio normale e non un comando
{
strcpy(Pacchetto->messaggio,message.messaggio);
write(sock,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
}
} 
break;

default:
while(1)
{
read(sock,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
read(sock,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));

if(strcmp(Pacchetto->nome_utente,"[SERVER]\0")==0)
{
    if(strcmp(Pacchetto->messaggio,"presente\0")==0) 
    printf("[SERVER] Stanza già esistente. Prova a spostarti con /sposta.\n");
    else if(strcmp(Pacchetto->messaggio,"Stanza creata con successo\0")==0) 
    printf("[SERVER] Stanza creata. Utilizza /sposta per spostarti.\n");
    else if(strcmp(Pacchetto->messaggio,"Errore sposta\0")==0)
    printf("[SERVER] La stanza non esiste o la password è errata.\n");
    else if(strcmp(Pacchetto->messaggio,"stanza cambiata\0")==0)
    printf("[SERVER] La stanza è stata cambiata con successo.\n");
    else if(strcmp(Pacchetto->messaggio,"Non sei admin\0")==0)
    printf("[SERVER] Attenzione: non sei Admin.\n");
    else if(strcmp(Pacchetto->messaggio,"fine\0")==0) //server per chiudere il processo padre rilasciando le risorse
    {
        free(Pacchetto); 
        close(sock);
        exit(EXIT_SUCCESS); 
    }
    else printf("%s: %s\n",Pacchetto->nome_utente,Pacchetto->messaggio); 
    //quest'ultimo serve per gestire eventuali messaggi di altro tipo provenienti da [SERVER]

}
else if(strcmp(Pacchetto->messaggio,"[Connesso]")==0) printf("[SERVER] %s si è connesso.\n",Pacchetto->nome_utente);
else if(strcmp(Pacchetto->messaggio,"[Disconnesso]")==0) printf("[SERVER] %s si è disconnesso.\n",Pacchetto->nome_utente);
else printf("%s: %s\n",Pacchetto->nome_utente,Pacchetto->messaggio);
}

}
}

