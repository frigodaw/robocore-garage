# robocore-garage
Aplikacja obsługująca automatyczny garaż wykonany z LEGO i Robocore na zaliczenie przedmiotu Mechatronika. Złożona jest z trzech podprogramów konsolowych:
- serwera przechowującego dane i zarządzajęcego przychodzącymi rozkazami,
- odpowiednika aplikacji mobilnej gdzie użytkownik może wybrać numer garażu i zaparkować,
- odpowiednika robocore'a obsługującego serwomechanizmy odpowiedzialne za ruch platfromy.

Rzeczywiste programy na robocore i telefon zostały wykonane przez inne osoby z grupy wobec czego nie są tu zamieszczone.

Do uruchomienia aplikacji konieczne są trzy terminale. Na pierwszym uruchamiamy serwer poprzez wpisanie "./TCPserver port1 port2" i podanie żądanych numerów portów. Na drugim terminalu uruchamiamy konsolową wersję aplikacji mobilnej "./client1 localhost port1". Ostatni terminal to odpowiednik platformy Robocore "./client2 localhost port2". Na konsoli aplikacji mobilnej wpisujemy dane postaci _ _ _ _ _, 
gdzie pierwsze miejsce to numer parkingu, drugie wjazd (1) lub wyjazd (0) trzy kolejne to hasło (trzykrotnie cyfra numeru parkingu). Następnie na konsoli robocore symulujemy udaną akcję poprzez wpisanie 1 lub nieudaną poprzez 0.
