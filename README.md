# SO-P3
### Wątki i semafory

Projekt polega na implementacji części biblioteki do obsługi wątków z semaforami przy użyciu biblioteki ucontext. Prostsze rozwiązanie zakłada przeplatanie instrukcji programów wywołaniem funkcji schedule, która będzie odpowiadać za zmianę wykonywanego wątku. Pełne rozwiązanie obejmuje zastąpienie owego przeplecenia przez symulację przerwań za pomocą funkcji wywoływanej w równych odstępach czasu (poprzez funkcję setitimer z biblioteki sys/time).


Funkcje:
 - void threads\_init() - tworzy jednoelementową listę wątków z wątkiem głównym
 - int thread\_create(void func(void\*), void\* arg) - tworzy nowy wątek wykonujący funkcję wskazywaną przez func i zwraca jego identyfikator
 - void thread\_join(int) - czeka na zakończenie wątku
 - void thread\_sleep(int) - usypia wątek na podaną liczbę mikrosekund
 - void thread\_exit() - kończy wątek
 - void sem\_init(*sem\_t, int) - inicjalizuje semafor
 - void sem\_wait(*sem\_t)  - blokuje semafor
 - void sem\_signal(*sem\_t) - odblokowuje semafor

Dodatkowo: 
 - funkcja schedule i inne funkcje pomocnicze, odpowiadające za przełączanie wątków
 
 Typy:
 - thread\_t - wątek
 - sem\_t - semafor
