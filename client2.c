#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

//Funkcja błedów
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

//Funkcja przetwarzania wlasnych bledow
void cerror(const char* msg)
{ 
	fprintf(stderr,"%s\n",msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	//Deklaracja zmiennych
	int sockfd;
	int port_no;
	int n;
	char buffer[256];

	//Deklaracja struktur
	struct sockaddr_in server_address;
	struct hostent *server;

	//Sprawdzenie parametrow
	if (argc == 1) cerror("BLAD: Nie podano adresu hosta ani portu");
	if (argc == 2) cerror("BLAD: Nie podano portu");

	//Konwersja argument na int
	port_no = atoi(argv[2]);

	//Stworzenie socketa
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) error("BLAD otwarcia socketa");

	//Adres serwera
	server = gethostbyname(argv[1]);
	if(server == NULL){
		fprintf(stderr, "BLAD, nie ma hosta \n");
		exit(0);
	}
		
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(port_no);
	if(connect(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		error("BLAD polaczenia");
	bzero(buffer,256);
	
	//Rozpoczęcie komunikacji, odczyt informacji z serwera
	n = read(sockfd, buffer,255);
	if(n == -1) error("BLAD odczytu z socketa");
	printf("Otrzymany sygnal sterujacy: %s\n",buffer);
	bzero(buffer,256);

	//TU MA BYC KOD INFORMACJI ZWROTNEJ SUKCES/PORAZKA
	printf("Operacja zakonczona: ");
	fgets(buffer,255,stdin);
	n = write(sockfd,buffer,strlen(buffer));	
	if(n == -1) error("BLAD pisania do socketa");
	bzero(buffer,256);
	
	close(sockfd);
	return 0;


}
