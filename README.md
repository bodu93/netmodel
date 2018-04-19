#### netmodel: some network programming models

referenced from chapter 6 in [chenshuo's book](https://www.amazon.cn/dp/B00AYS2KL0/ref=sr_1_1?ie=UTF8&qid=1524114203&sr=8-1&keywords=muduo)

all codes are from chenshuo's book and [UNPv3 chinese version](https://book.douban.com/subject/26434583/)

**NOTE: all codes are no error handlings, just for teching or  for my personal recording purpose, thanks:)**

0. accept+read/write

1.  accept+fork
2.  accept+thread
3.  prefork
4.  pre threaded
5.  poll(reactor)
6.  reactor + thread-per-task
7.  reactor + worker thread
8.  reactor + thread pool
9.  reactors in threads
10. reactors in processes
11. reactors + thread pool(combination of model 8 and model 9)
