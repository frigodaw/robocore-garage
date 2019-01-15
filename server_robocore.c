#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

void error(const char *msg){	//funkcja błędu
	perror(msg);
}
void error_socket(const char *msg, int *sockfd, int *new_sockfd){	//funkcja błędu
	printf(msg);
	close(*sockfd);
	close(*new_sockfd);
}

void czytajPlik(FILE *plik, char *buffer, char* numer, char* zajetosc, char* klucz){
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

void piszPlik(FILE *plik, char numer, char zajetosc, char* klucz){
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
			else{				 			//zajęte
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

int set_interface_attribs(int fd, int speed){
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

void set_mincount(int fd, int mcount){
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error tcgetattr: %s\n", strerror(errno));
        return;
    }

    tty.c_cc[VMIN] = mcount ? 1 : 0;
    tty.c_cc[VTIME] = 5;        /* half second timer */

    if (tcsetattr(fd, TCSANOW, &tty) < 0)
        printf("Error tcsetattr: %s\n", strerror(errno));
}




int main(int argc, char *argv[])
{
	//Zmienne socket
	int sockfd;
	int new_sockfd;
	int port_no;
	int n;
	char buffer[256];		//0-numer_miejsca, 1-rozkaz, 2,3,4-klucz
	socklen_t client_length;		//rozmiar adresów
	
	struct sockaddr_in server_address, client_address;		//Deklaracja struktur socket
	if (argc == 0) error("BLAD, podaj numer portu!\n");		//Sprawdzenie parametrow
	sockfd = socket(AF_INET, SOCK_STREAM, 0);				//Stworzenie gniazda
	if(sockfd == -1) error("BLAD otwarcia socket - APLIKACJA\n");	//Sprawdzenie czy stworzono socket
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))) error("setsockopt(BLAD uzycia SO_REUSERADDR\n");
	port_no = atoi(argv[1]); 								//Przypisanie numeru portu
	bzero((char *)&server_address, sizeof(server_address)); //Wypełnienie zerami obszaru pamięci dla adresu serwera
	bzero((char *)&client_address, sizeof(client_address)); //Wypełnienie zerami obszaru pamięci dla adresu clienta
	server_address.sin_family = AF_INET;					//Przypisanie rodziny do server_address
	server_address.sin_addr.s_addr = INADDR_ANY;			//Przypisanie adresu do server_address
	server_address.sin_port = htons(port_no);				//Przypisanie portu do server_address
	if(bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)	//Bindowanie
	error("BLAD podczas bindowania server - APLIKACJA\n"); 
		
	//Pozostale zmienne
	FILE *database;
	char numer;
	char rozkazIn;
	char kluczIn[256];
	char rozkazOut;
	char potwierdzenieOut;
	char potwierdzenieIn;
	
	//Zmienne serial
    char *portname = "/dev/ttyS0";
    int fd;
    int wlen;
	unsigned char buf[4];
	int rdlen;
			
	//Import danych z pliku
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
		listen(sockfd,5);
		client_length = sizeof(client_address);
		new_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_length);
		if(new_sockfd == -1) error("BLAD przy akceptowaniu sockfd - APLIKACJA\n");
		
		//Zerowanie buforów
		bzero(kluczIn,256);
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
		n = write(new_sockfd,buffer,strlen(buffer));
		if(n == -1) error("BLAD zapisu do socket - APLIKACJA\n");
		bzero(buffer,256);

		//Odczyt danych z aplikacji
		n = read(new_sockfd, buffer, 5);
		if(n == -1) error("BLAD odczytu z socket - APLIKACJA\n");
		
		numer = buffer[0];
		rozkazIn = buffer[1];
		kluczIn[0] = buffer[2];
		kluczIn[1] = buffer[3];
		kluczIn[2] =  buffer[4];
		
		printf("Otrzymane dane z aplikacji: %s\n", buffer);
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

		//Otwarcie komunikacji szeregowej
		fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
		if (fd < 0) error("BLAD otwarcia portu szeregowego\n");
		set_interface_attribs(fd, B9600);
		
		//Wysłanie do robocore informacji dotyczących sterowania
		wlen = write(fd, buffer,strlen(buffer));
		if (wlen < 0) error("BLAD pisania do robocore\n");
		tcdrain(fd);    /* delay for output */
		bzero(buffer,256);
		
		//Odczyt potwierdzenia wjazdu/wyjazdu z garażu od robocore
		rdlen = read(fd, buffer, 2);
        if (rdlen > 0) {
            buffer[rdlen] = 0;
			printf("Otrzymane dane od robocore: %s\n", buffer);
        } 
		else if (rdlen < 0) error("BLAD odczytu z robocore\n");
		potwierdzenieIn = buffer[0];
		close(fd);
		
		//Dane o wolnych miejscach parkingowych
		buffer[1] = Parking1.zajetosc;
		buffer[2] = Parking2.zajetosc;
		buffer[3] = Parking3.zajetosc;
		buffer[4] = Parking4.zajetosc;
		buffer[5] = Parking5.zajetosc;
		buffer[6] = Parking6.zajetosc;
		printf("Wyslane dane : %s\n", buffer);
		
		//Wysłanie do aplikacji informacji o wolnych miejscach
		n = write(new_sockfd,buffer,strlen(buffer));
		if(n == -1) error("BLAD zapisu do socket - APLIKACJA\n");
		bzero(buffer,256);
		
		
		//Wysłanie do aplikacji informacji o zakończeniu akcji
		n = write(new_sockfd,buffer,strlen(buffer));
		if(n == -1) error("BLAD pisania do socket - APLIKACJA\n");
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
	close(new_sockfd);
	close(sockfd);




}
