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

struct my_message
{
    long int message_type;
    char messaggio[1024];
};

void Formatto_testo (char * text);

int main (void)
{
int msg_id;
struct my_message message;

msg_id = msgget ((key_t) 4444, 0666 | IPC_CREAT);

if(msg_id==-1)
{
    perror("errore msg_id del writer");
    exit(EXIT_FAILURE);
}

message.message_type=1;

do
{
printf("Inserisci testo: ");
fgets(message.messaggio,1024,stdin);
Formatto_testo(message.messaggio);
if(msgsnd(msg_id,(void *)&message,sizeof(message),0)==-1)
{
    perror("errore msgsnd del writer");
    exit(EXIT_FAILURE);
}
} while(strcmp(message.messaggio,"end")!=0);

if(msgctl(msg_id,IPC_RMID,NULL)==-1)
{
perror("Errore msgctl client");
exit(EXIT_FAILURE);
}

}

void Formatto_testo (char * text)
{
    int i;
    i=strlen(text);
    text[i-1]='\0';
}