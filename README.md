# Ser_Client

2019.3.22
select模型介绍：
1.概念理解：网络编程中常见的几个调用模式：同步（Sync）/异步（Async）,阻塞（Block）/非阻塞
	同步：所谓同步，就是在发出一个功能调用时，在没有得到结果前，该调用就不返回。当前调用处于激活状态，逻辑上没有返回
	异步：异步概念和同步相对。当一个异步过程调用发出后，调用者不会立刻得到结果。调用者在发出调用后可以继续做自己的事，被调用者通过状态、通知来通知调用者，或者通过回调函数处理这个调用。
	阻塞：阻塞调用是指调用结果返回前，当前线程会被挂起，不再分配时间片
	非阻塞：非阻塞和阻塞的概念相对，是指不能立刻得到借过前，该函数不会阻塞当前进程，而回立刻返回。
2.select模型
	int select (
	  int nfds,
	  fd_set FAR * readfds,
	  fd_set FAR * writefds,
	  fd_set FAR * exceptfds,
	  const struct timeval FAR * timeout
	);
	三个参数一个用于检查可读性（readfds），一个用于检查可写性（writefds），另一个用于例外数据（ excepfds）
	注意：select返回后，它会修改每个fd_set结构，删除那些不存在待决 I / O操作的套接字句柄。
		意味着select输入时集合中的socket如果没有IO操作，经过select后该socket会被删除，有IO操作的sock则会被保留下来
