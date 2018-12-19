# semafory
Laboratorium z przedmiotu Systemy Operacyjne, zadanie 3 - semafory


Stworzenie bufora cyklicznego jako zasobu współdzielonego. Wzajemne wykluczanie realizowane na semaforach. Problem ma być rozwiązywany na procesach. Warunki odczytu i zapisu z bufora:
- producent nie może przeszkadzać innym producentom, to znaczy jeżeli jeden z nich wykonuje 5 zapisów, drugi z nich może zapisać tylko wtedy gdy pierwszy skończy
- analogicznie dla konsumentów
- należy uniemożliwić czytanie z pustego bufora oraz pisania na pełnym
- producenci i konsumenci mogą działać naprzemiennie

Komplilacja: g++ -Wall final.cpp
Test: ./skrypt
Testowane na Ubuntu 18.04 LTS
