model-3: prefork
version 1: accept(in every process) without lock
version 2: accept(in every process) with file lock
version 3: accept(in every process) with mutex
version 4: accept in only one process, transfer connected socket file descriptor
to other processes
