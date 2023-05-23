// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
int front = 0;
int rear = 0;

MySemaphore *mutex = new MySemaphore(1);
MySemaphore *empty = new MySemaphore(BUFFER_SIZE);
MySemaphore *full = new MySemaphore(0);

void Producer(int semaphoreArgument) {
    printf("Producer Started!\n");
    MySemaphore *mySemaphore = (MySemaphore *) semaphoreArgument;

    while (true) {
        // 프로듀싱할 데이터를 생성한다.
        int data = 42;

        // 버퍼가 가득 차면 대기한다.
        empty->P();

        // 버퍼에 데이터를 추가한다.
        mutex->P();
        buffer[rear] = data;
        rear = (rear + 1) % BUFFER_SIZE;
        mutex->V();

        // 생산한 데이터를 출력한다.
        printf("[Producer] Produced data = %d\n", data);

        // 컨슈머가 데이터를 소비할 수 있도록 full 세마포어를 증가시킨다.
        full->V();

        // 현재 스케줄러가 비선점형으로 구현되어 있어, Yield 호출을 명시적으로 해 주어야 컨텍스트 스위치를 진행한다.
        currentThread->Yield();
    }
}

void Consumer(int semaphoreArgument) {
    printf("Consumer Started!\n");
    MySemaphore *mySemaphore = (MySemaphore *) semaphoreArgument;

    while (true) {
        // 버퍼가 비어있으면 대기한다.
        full->P();

        // 버퍼에서 데이터를 소비한다.
        mutex->P();
        int data = buffer[front];
        front = (front + 1) % BUFFER_SIZE;
        mutex->V();

        // 소비한 데이터를 출력한다.
        printf("[Consumer] Consumed data = %d\n", data);

        // 생산자가 데이터를 생산할 수 있도록 empty 세마포어를 증가시킨다.
        empty->V();

        // 현재 스케줄러가 비선점형으로 구현되어 있어, Yield 호출을 명시적으로 해 주어야 컨텍스트 스위치를 진행한다.
        currentThread->Yield();
    }
}

void ProducerConsumerTest() {
    MySemaphore *mySemaphore = new MySemaphore(1);

    Thread *t1 = new Thread("Producer");
    Thread *t2 = new Thread("Consumer");

    printf("Before Fork()\n");

    t1->Fork(Producer, (void *) mySemaphore);
    t2->Fork(Consumer, (void *) mySemaphore);

    printf("After Fork()\n");
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    ProducerConsumerTest();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

