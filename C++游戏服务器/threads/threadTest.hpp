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
//定义类
class Counter{
public:
    void addCount(){m_count++;}
    int count()const{return m_count;}
    Counter():m_count(0){}
private:
    std::atomic<int> m_count;//定义原子操作
//    atomic_uint32_t m_count;//也可以使用这种内部方式
};



//函数声明
void testThreadInit();
void interface();
#endif /* threadTest_hpp */
