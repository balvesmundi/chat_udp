#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

char host_ip[20],remote_ip[20],datagram[512], newKey[512], key[512];
short host_port, remote_port;
int host_sock, remote_sock;
int i;
struct sockaddr_in si_remote2;
socklen_t slen2 = sizeof(si_remote2);
struct sockaddr_in si_host, si_remote;
socklen_t slen = sizeof(si_remote);

void *readFunction(){
    while(1)
    {
        if(recvfrom(host_sock, datagram, 512, 0, (struct sockaddr*)&si_remote, &slen)==-1)
            printf("Recvfrom failed!\n");

        if(strcmp(datagram, key) != 0){
            printf("Remote: %s", datagram);

            if(sendto(remote_sock, key, 512, 0, (struct sockaddr*)&si_remote2, slen2)==-1)
                printf("Sendto failed in notification!\n");
        }
        else
            printf("Remote received this message!\n");         
    }
}

void *writeFunction(){
    while(1){
        fgets(datagram, 512, stdin);
        if(sendto(remote_sock, datagram, 512, 0, (struct sockaddr*)&si_remote2, slen2)==-1)
            printf("Sendto failed!\n");
    }
}

void sendKeyValue(){
    int randomnumber = rand()%((int) pow(10,512)); //Gera um número aleatório de 0 a 512 dígitos
    sprintf(key, "%i", randomnumber); //Converte para string

    //printf("Minha chave %s\n", key);

    // Envia a chave para estabelecimento da confirmação de recebimento de mensagem
    if(sendto(remote_sock, key, 512, 0, (struct sockaddr*)&si_remote2, slen2)==-1)
        printf("Sendto failed in setting the key!\n");
}

void readKeyValue(){
    if(recvfrom(host_sock, newKey, 512, 0, (struct sockaddr*)&si_remote, &slen)==-1)
        printf("Recvfrom failed in reading the newKey!\n");

    //printf("Chave recebida: %s\n", newKey); 

    if(strcmp(newKey, key) != 0){
        // Se as chaves forem diferentes, minha chave será agora a nova chave
        strcpy(key, newKey);
        if(sendto(remote_sock, key, 512, 0, (struct sockaddr*)&si_remote2, slen2)==-1)
            printf("Sendto failed in confirming the key!\n");
    }
    // Caso contrário, minha chave continua a mesma... foi apenas o remoto confirmando
}

int main()
{
    srand(time(NULL));
    char bufclean;

    printf("Enter your ip:");
	fgets(host_ip, sizeof(host_ip), stdin);
	printf("Enter the remote ip:");
	fgets(remote_ip, sizeof(remote_ip), stdin);
	printf("Enter your port:");
	scanf("%hd", &host_port);
	printf("Enter the remote port:");
	scanf("%hd", &remote_port);

    scanf("%c", &bufclean);

    // Abertura do socket de leitura
    if((host_sock=socket(PF_INET, SOCK_DGRAM, 0))==-1)
        printf("Socket failed!\n");

    memset((char *) &si_host, 0, sizeof(si_host));
    si_host.sin_family = AF_INET;
    si_host.sin_port = htons(host_port);
    si_host.sin_addr.s_addr = inet_addr(host_ip);

    if(bind(host_sock, (struct sockaddr*)&si_host, sizeof(si_host))==-1)
        printf("Bind failed!\n");

    // Abertura do socket de escrita
    if((remote_sock=socket(PF_INET, SOCK_DGRAM, 0))==-1)
        printf("Socket failed!\n");

    memset((char *) &si_remote2, 0, sizeof(si_remote2));
    si_remote2.sin_family = AF_INET;
    si_remote2.sin_port = htons(remote_port);
    if (inet_aton(remote_ip, &si_remote2.sin_addr)==0) {
        printf("Inet_aton failed!\n");
        exit(1);
    }

    // Estabelecimento da chave única de confirmação de recebimento de mensagem
    sendKeyValue();
    readKeyValue();

    pthread_t tid;

    pthread_create(&tid, NULL, readFunction, NULL);
    pthread_create(&tid, NULL, writeFunction, NULL);
    
    pthread_exit(NULL);

    close(host_sock);
    close(remote_sock); 

    return 0;
}
