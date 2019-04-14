//
//  threadTest.cpp
//  HelloMac
//
//  Created by jains on 2019/4/14.
//  Copyright © 2019年 jains. All rights reserved.
//

#include "threadTest.hpp"

template <typename Iter,typename Fun>
double visitRange(thread::id id,Iter iterBegin,Iter iterEnd,Fun func)
{
    auto curId = std::this_thread::get_id();
    if(id == curId)
    {
        std::cout<<curId<<"hello main thread"<<endl;
    }
    else{
        cout<<curId<<"hello work thread"<<endl;
        
    }
    double v=0;
    for(auto iter = iterBegin;iter!=iterEnd;iter++)
    {
        v+=func(*iter);
    }
    return v;
}


void printAll(int a,int b,int c)
{
    cout<<a<<" "<<b<<" "<<c<<endl;
}
///创建线程的几种方式
void testThreadInit()
{
    int a=3,b=4,c=5;
    std::thread t([=]()->void {printAll(a, b, c);});//值传递
    t.join();
    std::thread t1([&]()->void {printAll(a, b, c);});//引用传递
    t1.join();
    thread t2(printAll,a,b,c);//值传递
    t2.join();
    thread t3(printAll,ref(a),ref(b),ref(c));//引用传递
    t3.join();
}

int work(int a)
{
    return a+a;
}

template <class Iter>
void realWork(Counter &c,double &totalValue,Iter b,Iter e)
{
    for(;b!=e;++b)
    {
        totalValue+=work(*b);
        c.addCount();
    }
}
void add(int a,int b,int &c)
{
    c=a+b;
}
bool printStep(Counter &c,int maxCount)
{
    auto C = c.count();
    if(C==maxCount)
    {
        cout<<"ok,finished"<<endl;
        return true;
    }
    return false;
}
void interface()
{
    unsigned int n = thread::hardware_concurrency();//支持的线程数
    cout<<n<<" concurrent threads are supported"<<endl;
    vector<int>vec;
    double totalValue = 0;
    for(int i=0;i<10000000;i++)
    {
        vec.push_back(rand()%10);
    }
    Counter counter;
    realWork(counter, totalValue, vec.begin(), vec.end());
    cout<<"total times: "<<counter.count()<<" "<<totalValue<<endl;
    
    totalValue = 0;
    Counter counter2;
    thread printCount([&counter2](){//不断的来判断是否计算完毕
        while(!printStep(counter2,10000000));
    });
    
    
    ///接下来三个线程操作的是同一个计数器对象counter2，给对象设置为原子类型操作
    auto iter  = vec.begin() +(vec.size()/3);
    auto iter2 = vec.begin()+(vec.size()/3*2);
    thread b([&counter2,&totalValue,iter,iter2]()->void{//计算1/3--2/3之间的值
        realWork(counter2, totalValue, iter, iter2);
    });
    auto end = vec.end();
    double totalC=0;
    thread c([&counter2,&totalC,iter2,end]{//计算最后1/3的值
        realWork(counter2, totalC, iter2, end);
    });
    
    double totalM =0;
    realWork(counter2, totalM, vec.begin(), iter);//主线程计算开始1/3之间的
    b.join();
    c.join();
    auto realTotalCount = counter2.count();
    totalValue +=totalC+totalM;
    cout<<"total times use multithread: "<<realTotalCount<<" "<<totalValue<<endl;
    
    printCount.join();
}

//thread 的两种死法
void joinWorker()
{
    cout<<"hello world"<<endl;
}
void threadDied()
{
    thread j(joinWorker);
    //    thread j1(ff);
    //    j1.join();//阻塞模式，子线程执行完毕后，主线程才继续执行.thread::join()会清理子线程相关的内存空间，此后thread object将不再和这个子线程相关了，即thread object不再joinable了，所以join对于一个子线程来说只可以被调用一次
    //    j.join();
    cout<<j.joinable()<<endl;
    j.detach();//非阻塞模式，子线程分离，子和主两者同时进行
    cout<<"main"<<endl;
    this_thread::sleep_for(std::chrono::seconds(7));
    cout<<"over"<<endl;
}
