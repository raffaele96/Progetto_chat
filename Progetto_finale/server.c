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
#include <sys/stat.h>
#define MAX_CLIENTS 5

struct Pacchetto_client
{
    char nome_utente[1024];
    char messaggio[1024];
    char password[1024];
    char stanza[1024];
    char password_stanza[1024];
};

void Formatto_testo (char * text);

int main (void)
{
struct sockaddr_in indirizzo_server,indirizzo_client;
int client_len,sock,client_sockets[MAX_CLIENTS],sd,res,max_sd,cont,fd;
struct Pacchetto_client *Pacchetto;
fd_set readfds;
fd_set writefds;
char client_sockets_name[MAX_CLIENTS][1024],password[1024],aux[1024],client_sockets_room[MAX_CLIENTS][1024];
struct stat information;
FILE *fp;

Pacchetto=(struct Pacchetto_client*)malloc(sizeof(struct Pacchetto_client));

if(Pacchetto==NULL)
{
perror("[SERVER] Impossibile allocare memoria.");
exit(EXIT_FAILURE);
}

sock=socket(AF_INET,SOCK_STREAM,0);
if(sock==-1)
{
    perror("Errore creazione socket server");
    exit(EXIT_FAILURE);
}

indirizzo_server.sin_family=AF_INET;
indirizzo_server.sin_port=htons(2222);
indirizzo_server.sin_addr.s_addr=INADDR_ANY;

if(bind(sock,(struct sockaddr*)&indirizzo_server,sizeof(indirizzo_server))==-1)
{
perror("[SERVER] Errore bind");
exit(EXIT_FAILURE);
}

if(listen(sock,5)==-1)
{
perror("[SERVER] Errore listen");
exit(EXIT_FAILURE);
}

for(sd=0;sd<MAX_CLIENTS;sd++) client_sockets[sd]=0;

 while(1)
    {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        max_sd = sock;
        
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            sd = client_sockets[i];
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd)  max_sd = sd;
        }
        
        res = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ( res < 0) 
        {
            perror("[SERVER] Errore select");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(sock, &readfds)) 
        { 
            sd = accept(sock, NULL, 0);
            if (sd < 0) 
            {
                perror("error : cannot accept client socket\n");
                exit(EXIT_FAILURE);
            }
            
            for (i = 0; i < MAX_CLIENTS; i++) 
            {
                if (client_sockets[i] == 0)
                 {
                     res=0;
                     client_sockets[i] = sd;
                     read(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                     read(sd,client_sockets_name[i],sizeof(client_sockets_name[i]));
                     read(sd,Pacchetto->password,sizeof(Pacchetto->password));
                     fp=fopen("nome_utenti.txt","a+");
                     stat("nome_utenti.txt",&information);
                     if(information.st_size==0)
                     {
                        fputs("ADMIN",fp);
                        fputs("\n",fp);
                        fputs("ROOT",fp);
                        fputs("\n",fp);
                     }
                     rewind(fp);
                     while(fgets(Pacchetto->nome_utente,1024,fp)!=NULL)
                     {
                         Formatto_testo(Pacchetto->nome_utente);
                         if(strcmp(Pacchetto->nome_utente,client_sockets_name[i])==0) 
                         {
                             res=1;
                             fgets(password,1024,fp);
                             Formatto_testo(password);
                             break;
                         }
                         fgets(aux,1024,fp); // serve a fare scorrere il puntatore 
                     }
                     rewind(fp);
                     if(res==0)
                     {
                     if(strcmp(Pacchetto->messaggio,"registrazione\0")==0)
                     {
                        strcpy(Pacchetto->messaggio,"ok\0");
                        write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                        fputs(client_sockets_name[i],fp);
                        fputs("\n",fp);
                        fputs(Pacchetto->password,fp);
                        fputs("\n",fp);
                     }
                     if(strcmp(Pacchetto->messaggio,"login\0")==0)
                     {
                        strcpy(Pacchetto->messaggio,"notk\0");
                        write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                        client_sockets[i]=0;
                    }
                    }

                    if(res==1)
                    {
                    if(strcmp(Pacchetto->messaggio,"registrazione\0")==0)
                    {
                    strcpy(Pacchetto->messaggio,"notk\0");
                    write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                    client_sockets[i]=0;
                    }
                    if(strcmp(Pacchetto->messaggio,"login\0")==0)
                    {
                        if(strcmp(Pacchetto->password,password)==0)
                        {
                        strcpy(Pacchetto->messaggio,"ok\0");
                        write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio)); 
                        }
                        else 
                        {
                         strcpy(Pacchetto->messaggio,"notk\0");
                         write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                        client_sockets[i]=0;
                        }
                    }

                    }

                    fclose(fp);
                  

                    if(strcmp(Pacchetto->messaggio,"notk\0")!=0)
                 {
                    strcpy(Pacchetto->stanza,"Pubblica\0");
                    strcpy(client_sockets_room[i],"Pubblica");
                    for(cont=0;cont<MAX_CLIENTS;cont++)
                    {
                    if(client_sockets[cont]!=0 && strcmp(client_sockets_room[cont],client_sockets_room[i])==0) 
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],"[Connesso]",sizeof("[connesso]"));
                    }
                    }
                 }
                     break;
                }
            }
            if(i==MAX_CLIENTS) close(sd);
        }

        for (i = 0; i < MAX_CLIENTS; i++) 
        {
            sd = client_sockets[i];
            if (FD_ISSET(sd, &readfds)) 
            {
                res = read(sd, Pacchetto->messaggio, sizeof(Pacchetto->messaggio));
                if (res == 0) 
                {
                    client_sockets[i] = 0;
                    printf("close socket\n");
                     for(cont=0;cont<MAX_CLIENTS;cont++)
                    {
                    if(client_sockets[cont]!=0 && strcmp(client_sockets_room[cont],client_sockets_room[i])==0)
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],"[Disconnesso]",sizeof("[Disconnesso]"));
                    }
                    }
                    strcpy(client_sockets_name[i]," ");
                    strcpy(client_sockets_room[i]," ");


                } 
                if(res>0) 
                {
                res=0;
                if(strcmp(Pacchetto->messaggio,"/crea_stanza\0")==0 || strcmp(Pacchetto->messaggio,"/sposta\0")==0)
                {
                    read(sd,Pacchetto->stanza,sizeof(Pacchetto->stanza));
                    read(sd,Pacchetto->password_stanza,sizeof(Pacchetto->password_stanza));
                    fp=fopen("stanze.txt","a+");
                     stat("stanze.txt",&information);
                     if(information.st_size==0)
                     {
                        fputs("Pubblica",fp);
                        fputs("\n",fp);
                        fputs("\n",fp);
                     }
                     rewind(fp);
                     while(fgets(aux,1024,fp)!=NULL)
                     {
                         Formatto_testo(aux);
                         if(strcmp(Pacchetto->stanza,aux)==0) 
                         {
                             res=1;
                             fgets(password,1024,fp);
                             Formatto_testo(password);
                             break;
                         }
                         fgets(password,1024,fp); // serve a fare scorrere il puntatore 
                     }
                rewind(fp);
                if(res==0 && strcmp(Pacchetto->messaggio,"/crea_stanza\0")==0)
                {
                        strcpy(Pacchetto->messaggio,"Stanza creata con successo\0");
                        fputs(Pacchetto->stanza,fp);
                        fputs("\n",fp);
                        fputs(Pacchetto->password_stanza,fp);
                        fputs("\n",fp);
                        strcpy(Pacchetto->nome_utente,"[SERVER]");
                        write(sd,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                        write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }

                if(res==0 && strcmp(Pacchetto->messaggio,"/sposta\0")==0)
                {
                printf("prova\n");
                strcpy(Pacchetto->messaggio,"Errore sposta\0");
                strcpy(Pacchetto->nome_utente,"[SERVER]");
                write(sd,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }

                if(res==1 && strcmp(Pacchetto->messaggio,"/crea_stanza\0")==0)
                {
                    strcpy(Pacchetto->messaggio,"presente\0");
                    strcpy(Pacchetto->nome_utente,"[SERVER]");
                    write(sd,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                    write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }

                if(res==1 && strcmp(Pacchetto->messaggio,"/sposta\0")==0)
                {
                if(strcmp(password,Pacchetto->password_stanza)==0)
                {
                    strcpy(aux,client_sockets_room[i]);
                    strcpy(Pacchetto->messaggio,"stanza cambiata\0");
                    strcpy(Pacchetto->nome_utente,"[SERVER]");
                    write(sd,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                    write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                    strcpy(client_sockets_room[i],Pacchetto->stanza);
                     for(cont=0;cont<MAX_CLIENTS;cont++)
                    {
                    if(client_sockets[cont]!=0 && strcmp(client_sockets_room[cont],client_sockets_room[i])==0) 
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],"[Connesso]",sizeof("[connesso]"));
                    }
                    }
                     
                    for(cont=0;cont<MAX_CLIENTS;cont++)
                    {
                    if(client_sockets[cont]!=0 && strcmp(client_sockets_room[cont],aux)==0)
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],"[Disconnesso]",sizeof("[Disconnesso]"));
                    }
                    }
                }
                else
                {
                    strcpy(Pacchetto->messaggio,"Errore sposta\0");
                    strcpy(Pacchetto->nome_utente,"[SERVER]");
                    write(sd,Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                    write(sd,Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }
                }
                fclose(fp);
                }

                else if(strcmp(Pacchetto->messaggio,"/stanza_attuale\0")==0)
                {
                    sprintf(Pacchetto->messaggio,"La stanza attuale è: %s",client_sockets_room[i]);
                    strcpy(Pacchetto->nome_utente,"[SERVER]");
                    write(client_sockets[i],Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                    write(client_sockets[i],Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }

                else if(strcmp(Pacchetto->messaggio,"/broadcast\0")==0)
                {
                if(strcmp(client_sockets_name[i],"ADMIN")==0)
                  {
                read(client_sockets[i], Pacchetto->messaggio, sizeof(Pacchetto->messaggio));
                 for(cont=0;cont<MAX_CLIENTS;cont++)
                    {
                    if(client_sockets[cont]!=0) 
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                    }
                    }
                  }
                else
                  {
                strcpy(Pacchetto->nome_utente,"[SERVER]");
                strcpy(Pacchetto->messaggio,"Non sei admin\0");
                write(client_sockets[i],Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                write(client_sockets[i],Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                  }
                }

                else if(strcmp(Pacchetto->messaggio,"end\0")==0)
                {
                strcpy(Pacchetto->nome_utente,"[SERVER]");
                strcpy(Pacchetto->messaggio,"fine\0");
                write(client_sockets[i],Pacchetto->nome_utente,sizeof(Pacchetto->nome_utente));
                write(client_sockets[i],Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                }


                else
                {
                for(cont=0;cont<MAX_CLIENTS;cont++)
                {
                    if(client_sockets[cont]!=0 && strcmp(client_sockets_room[cont],client_sockets_room[i])==0) 
                    {
                        write(client_sockets[cont],client_sockets_name[i],sizeof(client_sockets_name[i]));
                        write(client_sockets[cont],Pacchetto->messaggio,sizeof(Pacchetto->messaggio));
                    }
                }
                }
                }
                if(res<0) exit(EXIT_FAILURE);
                
            }
        }
    }

free(Pacchetto);


}



void Formatto_testo (char * text)
{
    int i;
    i=strlen(text);
    text[i-1]='\0';
}