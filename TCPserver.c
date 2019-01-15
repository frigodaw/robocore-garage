#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg){	//funkcja błędu
	perror(msg);
	exit(1);
}

void czytajPlik(FILE *plik, char *buffer, char* numer, char* zajetosc, char *klucz){
	fgets(buffer, 256, plik);
	*numer = buffer[0];
	fgets(buffer, 256, plik);
	*zajetosc = buffer[0];
	fgets(buffer, 256, plik);
	strncpy(klucz, buffer, 256);
	klucz[3] = '\0';
	fgets(buffer, 256, plik);
	bzero(buffer, 256);
}

void piszPlik(FILE *plik, char numer, char zajetosc, char *klucz){
	fprintf(plik, "%c\n", numer);
	fprintf(plik, "%c\n", zajetosc);
	fprintf(plik, "%s\n\n", klucz);
}

void parkowanie(char rozkazIn, char* kluczIn, char* zajetoscParking, char* kluczParking, char* rozkazOut,  char* potwierdzenieOut){
	printf("kluczParking: %s\n", kluczParking);
	printf("kluczIn: %s\n", kluczIn);

	if(rozkazIn == '0'){				//wyjazd
		*rozkazOut = '0';
		if(strcmp(kluczIn, kluczParking) == 0){		//klucze zgodne
			if(*zajetoscParking == '1'){			//na parkingu jest samochod
				printf("Klucz poprawny, zgoda na wyjazd\n");
				*potwierdzenieOut = '1';
				*zajetoscParking = '0';		
			}
			else {
				printf("Na parkingu nie ma auta!\n");
				*potwierdzenieOut = '0';
			}
		}
		else{	//klucz niepoprawny		
			printf("Klucz niepoprawny, brak zgody na wyjazd\n");
			*potwierdzenieOut = '0';	
		}
	}
	
	else if(rozkazIn == '1'){ 			//wjazd
		*rozkazOut = '1';
		if(strcmp(kluczIn, kluczParking) == 0){		//klucze zgodne
			if(*zajetoscParking == '0'){ 	//miejsce wolne
				printf("Miejsce wolne, zgoda na wjazd\n");
				*potwierdzenieOut = '1';
				*zajetoscParking = '1';	
			}
			else{				 			//zajête
				printf("Miejsce zajete, brak zgody na wjazd\n");
				*potwierdzenieOut = '0';
			}
		}
		else{	//klucz niepoprawny		
			printf("Klucz niepoprawny, brak zgody na wjazd\n");
			*potwierdzenieOut = '0';	
		}
	}	
}




int main(int argc, char *argv[])
{
	//Zmienne socket
	int sockfd1, sockfd2;
	int new_sockfd1, new_sockfd2;
	int port_no1, port_no2;
	int n1, n2;
	char buffer[256];		//0-numer_miejsca, 1-rozkaz, 2-klucz
	socklen_t client_length1, client_length2;	//rozmiar adresów
	
	//Deklaracja struktur socket
	struct sockaddr_in server_address1, server_address2, client_address1, client_address2;

	//Sprawdzenie parametrow
	if (argc == 0) error("BLAD, podaj numery obu portow!");
	if (argc == 1) error("BLAD, podaj numer drugiego portu!");
	
	//Stworzenie gniazda
	sockfd1 = socket(AF_INET, SOCK_STREAM, 0);
	sockfd2 = socket(AF_INET, SOCK_STREAM, 0);

	//Sprawdzenie czy stworzono socket
	if(sockfd1 == -1) error("BLAD otwarcia socket1 - APLIKACJA");
	if(sockfd2 == -1) error("BLAD otwarcia socket2 - ROBOCORE");
	if(setsockopt(sockfd1,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))) error("setsockopt(BLAD uzycia SO_REUSERADDR\n");
	if(setsockopt(sockfd2,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))) error("setsockopt(BLAD uzycia SO_REUSERADDR\n");	

	//Czynności przygotowujące do bindowania
	port_no1 = atoi(argv[1]); 				//Przypisanie numeru portu
	port_no2 = atoi(argv[2]); 				//Przypisanie numeru portu
	bzero((char *)&server_address1, sizeof(server_address1)); //Wypełnienie zerami obszaru pamięci dla adresu serwera1
	bzero((char *)&server_address2, sizeof(server_address2)); //Wypełnienie zerami obszaru pamięci dla adresu serwera2
	bzero((char *)&client_address1, sizeof(client_address1)); //Wypełnienie zerami obszaru pamięci dla adresu clienta1
	bzero((char *)&client_address2, sizeof(client_address2)); //Wypełnienie zerami obszaru pamięci dla adresu clienta2

	server_address1.sin_family = AF_INET;			//Przypisanie rodziny do server_address1
	server_address1.sin_addr.s_addr = INADDR_ANY;	//Przypisanie adresu do server_address1
	server_address1.sin_port = htons(port_no1);		//Przypisanie portu do server_address1
	server_address2.sin_family = AF_INET;			//Przypisanie rodziny do server_address2
	server_address2.sin_addr.s_addr = INADDR_ANY;	//Przypisanie adresu do server_address2
	server_address2.sin_port = htons(port_no2);		//Przypisanie portu do server_address2

	if(bind(sockfd1, (struct sockaddr *) &server_address1, sizeof(server_address1)) < 0)
	error("BLAD podczas bindowania server1 - APLIKACJA"); 
	if(bind(sockfd2, (struct sockaddr *) &server_address2, sizeof(server_address2)) < 0)
	error("BLAD podczas bindowania server2 - ROBOCORE"); 
	
	
	//Dane
	FILE *database;
	char numer;
	char rozkazIn;
	char kluczIn[256];
	char rozkazOut;
	char potwierdzenieOut;
	char potwierdzenieIn;
	
	
	//Impport danych z pliku
	struct parking{
		char numer;
		char zajetosc;	//0-wolne, 1-zajęte
		char klucz[256];
	};
	
	struct parking Parking1;
	struct parking Parking2;
	struct parking Parking3;
	struct parking Parking4;
	struct parking Parking5;
	struct parking Parking6;
	

	//Wpisanie danych pliku do struktury parking
	if ((database=fopen("database.txt", "r")) != NULL){
		czytajPlik(database, buffer, &Parking1.numer, &Parking1.zajetosc, Parking1.klucz);
		czytajPlik(database, buffer, &Parking2.numer, &Parking2.zajetosc, Parking2.klucz);
		czytajPlik(database, buffer, &Parking3.numer, &Parking3.zajetosc, Parking3.klucz);
		czytajPlik(database, buffer, &Parking4.numer, &Parking4.zajetosc, Parking4.klucz);
		czytajPlik(database, buffer, &Parking5.numer, &Parking5.zajetosc, Parking5.klucz);
		czytajPlik(database, buffer, &Parking6.numer, &Parking6.zajetosc, Parking6.klucz);
		fclose(database);
	}
	else error("BLAD otwarcia bazy danych\n");
	
	

	while(1) {
		//Tworzy nowy socket dla pierwszego nadchodzącego połączenia - APLIKACJA
		listen(sockfd1,5);
		client_length1 = sizeof(client_address1);
		new_sockfd1 = accept(sockfd1, (struct sockaddr *) &client_address1, &client_length1);
		if(new_sockfd1 == -1) error("BLAD przy akceptowaniu sockfd1 - APLIKACJA");
		
		//Tworzy nowy socket dla drugiego nadchodzącego połączenia - ROBOCORE
		listen(sockfd2,5);
		client_length2 = sizeof(client_address2);
		new_sockfd2 = accept(sockfd2, (struct sockaddr *) &client_address2, &client_length2);
		if(new_sockfd2 == -1) error("BLAD przy akceptowaniu sockfd2 - ROBOCORE");
		
		//Zerowanie buforów
		bzero(kluczIn,256);
		bzero(buffer,256);
		bzero(buffer,256);
		
		//Dane o wolnych miejscach parkingowych
		buffer[0] = 'P';
		buffer[1] = Parking1.zajetosc;
		buffer[2] = Parking2.zajetosc;
		buffer[3] = Parking3.zajetosc;
		buffer[4] = Parking4.zajetosc;
		buffer[5] = Parking5.zajetosc;
		buffer[6] = Parking6.zajetosc;
		printf("Wyslane dane : %s\n", buffer);
		
		//Wysłanie do aplikacji informacji o wolnych miejscach
		n1 = write(new_sockfd1,buffer,strlen(buffer));
		if(n1 == -1) error("BLAD pisania do socket1 - APLIKACJA");
		bzero(buffer,256);

		//Odczyt danych z aplikacji
		n1 = read(new_sockfd1, buffer, 255);
		if(n1 == -1) error("BLAD czytania z socket1 - APLIKACJA");
		
		numer = buffer[0];
		rozkazIn = buffer[1];
		kluczIn[0] = buffer[2];
		kluczIn[1] = buffer[3];
		kluczIn[2] =  buffer[4];
		
		printf("Otrzymane dane z aplikacji: %s", buffer);
		printf("Numer miejsca: %c \n", numer);
		printf("Rozkaz: %c \n", rozkazIn);		//0-wyjazd, 1-parkuj
		printf("Klucz: %s \n", kluczIn);
		bzero(buffer,256);
		
		switch(numer){
			case '1': 
				parkowanie(rozkazIn, kluczIn, &Parking1.zajetosc, Parking1.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			case '2':
				parkowanie(rozkazIn, kluczIn, &Parking2.zajetosc, Parking2.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			case '3':
				parkowanie(rozkazIn, kluczIn, &Parking3.zajetosc, Parking3.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			case '4':
				parkowanie(rozkazIn, kluczIn, &Parking4.zajetosc, Parking4.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			case '5':
				parkowanie(rozkazIn, kluczIn, &Parking5.zajetosc, Parking5.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			case '6':
				parkowanie(rozkazIn, kluczIn, &Parking6.zajetosc, Parking6.klucz, &rozkazOut, &potwierdzenieOut);
			break;
			
			default:
				printf("Uzytkownik rozlaczyl sie!\n\n");
				continue;
			break;
		} 
		
		buffer[0] = numer;
		buffer[1] = rozkazOut;
		buffer[2] = potwierdzenieOut;
		printf("KOMENDY WYJSCIOWE \n");
		printf("Numer: %c\n", numer);
		printf("Rozkaz: %c\n", rozkazOut);
		printf("Potwierdzenie: %c\n", potwierdzenieOut);
		printf("Wyslane dane do robocore: %s\n", buffer);
		printf("Oczekiwanie na wykonanie akcji przez robocore...\n");
		
		//Wysłanie do robocore informacji dotyczących sterowania
		n2 = write(new_sockfd2,buffer,strlen(buffer));
		if(n2 == -1) error("BLAD pisania do socket2 - ROBOCORE");
		bzero(buffer,256);
		
		//Odczyt potwierdzenia wjazdu/wyjazdu z garażu od robocore
		n2 = read(new_sockfd2, buffer, 255);
		if(n2 == -1) error("BLAD czytania z socket2 - ROBOCORE");
		printf("Otrzymane dane od robocore: %s", buffer);
		potwierdzenieIn = buffer[0];
				
		//Dane o wolnych miejscach parkingowych
		buffer[1] = Parking1.zajetosc;
		buffer[2] = Parking2.zajetosc;
		buffer[3] = Parking3.zajetosc;
		buffer[4] = Parking4.zajetosc;
		buffer[5] = Parking5.zajetosc;
		buffer[6] = Parking6.zajetosc;
		printf("Wyslane dane do aplikacji: %s\n", buffer);
		
		//Wysłanie do aplikacji informacji zakończeniu akcji
		n1 = write(new_sockfd1,buffer,strlen(buffer));
		if(n1 == -1) error("BLAD pisania do socket1 - APLIKACJA\n\n");
		bzero(buffer,256);
		
				
		//Synchronizacja danych zgodnie z sygnalem zwrotnym od robocore
		if (potwierdzenieIn == '1'){		//akcja udana
			//Zapis nowych danych do pliku	
			if ((database=fopen("database.txt", "w")) != NULL){
				piszPlik(database, Parking1.numer, Parking1.zajetosc, Parking1.klucz);
				piszPlik(database, Parking2.numer, Parking2.zajetosc, Parking2.klucz);
				piszPlik(database, Parking3.numer, Parking3.zajetosc, Parking3.klucz);
				piszPlik(database, Parking4.numer, Parking4.zajetosc, Parking4.klucz);
				piszPlik(database, Parking5.numer, Parking5.zajetosc, Parking5.klucz);
				piszPlik(database, Parking6.numer, Parking6.zajetosc, Parking6.klucz);
				fclose(database);
			}
			else error("BLAD otwarcia bazy danych do zapisu\n");	
		}
		
		else{		//akcja nieudana
			if ((database=fopen("database.txt", "r")) != NULL){
				czytajPlik(database, buffer, &Parking1.numer, &Parking1.zajetosc, Parking1.klucz);
				czytajPlik(database, buffer, &Parking2.numer, &Parking2.zajetosc, Parking2.klucz);
				czytajPlik(database, buffer, &Parking3.numer, &Parking3.zajetosc, Parking3.klucz);
				czytajPlik(database, buffer, &Parking4.numer, &Parking4.zajetosc, Parking4.klucz);
				czytajPlik(database, buffer, &Parking5.numer, &Parking5.zajetosc, Parking5.klucz);
				czytajPlik(database, buffer, &Parking6.numer, &Parking6.zajetosc, Parking6.klucz);
				fclose(database);
			}
			else error("BLAD otwarcia bazy danych do odczytu\n");
		}
		printf("Aktualny stan parkingu: P%c%c%c%c%c%c\n\n", Parking1.zajetosc, Parking2.zajetosc, Parking3.zajetosc, Parking4.zajetosc, Parking5.zajetosc, Parking6.zajetosc);
	}	

	//Zamknięcie połączenia
	close(new_sockfd1);
	close(sockfd1);
	close(new_sockfd2);
	close(sockfd2);




}
