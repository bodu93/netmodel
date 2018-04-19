##single thread reactor(select/poll/epoll/kqueue/...)

using io mutiplexing mechanism as backend to wait for multiple file descriptors, we can reuse the only one thread of control.
