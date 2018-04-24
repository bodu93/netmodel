## model-3: prefork(Apache model)

using prefork processed, we accept(2) in every process or not(accept in one process and transfer to other processes).

when accept(2) in every process, we should lock accept(2) call, forbid the [thundering herd problem](https://en.wikipedia.org/wiki/Thundering_herd_problem)

* version 1: accept(in every process) without lock
* version 2: accept(in every process) with file lock
* version 3: accept(in every process) with pthread mutex(using pthread mutex attribute `PTHREAD_PROCESS_SHARED`)
* version 4: accept in only one process, then transfer connected socket file descriptor to other processes