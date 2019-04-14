//
//  threadTest.hpp
//  HelloMac
//
//  Created by jains on 2019/4/14.
//  Copyright © 2019年 jains. All rights reserved.
//

#ifndef threadTest_hpp
#define threadTest_hpp

#include <stdio.h>
#include <iostream>
using namespace std;
#include<thread>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <future>
#include <vector>
#include <mutex>
#include <list>
//定义类
class Counter{
public:
    void addCount(){m_count++;}
    
    void addResource(int r){m_totalResource++;}
    
    int count()const{return m_count;}
    int aveResource(){
        if(m_count==0)
            return 1;
        else
            return m_totalResource / m_count;
        }
    Counter():m_count(0),m_totalResource(0){}
    
    void lockMutex(){m_mutex.lock();}
    void unlockMutex(){m_mutex.unlock();}
private:
    atomic<int> m_count;//定义原子操作
    //auto<int>类型，跟int类型基本一样，可以操作很多++ -- + - * /等都支持
//    atomic_int m_count;//也可以使用这种内部方式
    atomic_int m_totalResource;
    mutex m_mutex;//互斥量，互斥访问
    
};

//函数声明
//void testThreadInit();
//void interface();
//void threadDied();//thread的两种死法
void threadConnect2();//线程间交互2
#endif /* threadTest_hpp */
