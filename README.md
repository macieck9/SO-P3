# SO-P3
#### Implementacja części biblioteki do obsługi wątków z semaforami przy użyciu biblioteki ucontext
Funkcje:
 - thread\_t thread\_create(void func(void\*), void\* arg) - tworzy nowy wątek wykonujący funkcję wskazywaną przez func i zwraca go
 - void thread\_join(thread\_t) - czeka na zakończenie wątku
 - void thread\_exit() - kończy wątek
 - void sem\_wait(sem\_t)  - blokuje semafor
 - void sem\_signal(sem\_t) - odblokowuje semafor
