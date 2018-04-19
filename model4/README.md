##model 4: pre threaded

using precreated threads, we accept(2) in every thread or not(accept in one thread and transfer to other threads).

when accept(2) in every thread, we should lock accept(2) call for forbidding the [thundering herd problem](https://en.wikipedia.org/wiki/Thundering_herd_problem)

* version 1: accept in every thread
* version 2: accept in main thread and transfer connected fd to other threads