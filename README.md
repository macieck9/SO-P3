# SO-P3
### Wątki i semafory

Projekt polegać będzie na implementacji części biblioteki do obsługi wątków z semaforami przy użyciu biblioteki ucontext. Prostsze rozwiązanie, ktore prawdopodobnie początkowo zaimplementujemy, zakłada przeplatanie instrukcji programów wywołaniem funkcji schedule, która będzie odpowiadać za zmianę wykonywanego wątku. Pełne rozwiązanie obejmuje zastąpienie owego przeplecenia przez symulację przerwań za pomocą funkcji wywoływanej w równych odstępach czasu (np. poprzez funkcję alarm z biblioteki unistd).


Funkcje:
 - void init() - tworzy jednoelementową listę wątków z wątkiem głównym
 - int thread\_create(void func(void\*), void\* arg) - tworzy nowy wątek wykonujący funkcję wskazywaną przez func i zwraca jego identyfikator
 - void thread\_join(int) - czeka na zakończenie wątku
 - void thread\_exit() - kończy wątek
 - void sem\_wait(sem\_t)  - blokuje semafor
 - void sem\_signal(sem\_t) - odblokowuje semafor

Dodatkowo: 
 - funkcja schedule / jej odpowiednik w wersji z przerwaniami - odpowiada za przełączanie wątków
 
 Typy:
 - thread\_t - wątek
 - sem\_t - semafor
