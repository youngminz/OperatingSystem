// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {}
Lock::~Lock() {}
void Lock::Acquire() {}
void Lock::Release() {}

Condition::Condition(char* debugName) { }
Condition::~Condition() { }
void Condition::Wait(Lock* conditionLock) { ASSERT(FALSE); }
void Condition::Signal(Lock* conditionLock) { }
void Condition::Broadcast(Lock* conditionLock) { }

MySemaphore::MySemaphore(int initialValue) {
    this->value = initialValue;
    this->queue = new List();
}

void MySemaphore::P() {
    // 인터럽트를 비활성화 시킴으로서 P() 함수를 실행하는 동안 컨텍스트 스위칭이 일어나지 않도록 한다.
    IntStatus prevIntLevel = interrupt->SetLevel(IntOff);

    // 세마포어 값을 감소한다.
    this->value--;

    // 만약 현재 세마포어가 사용 불가능한 상태인 경우,
    if (value < 0) {

        // 큐에 현재 실행 중인 쓰레드를 집어넣는다.
        this->queue->Append(currentThread);

        // Sleep() 함수는 인터럽트가 이미 비활성화 되어 있다고 가정한다. 
        // 이는 이 함수가 원자성을 위해 인터럽트를 반드시 비활성화해야 하는 동기화 루틴에서 호출되기 때문이다.
        // 만약 인터럽트를 다시 활성화하면, `ASSERT(interrupt->getLevel() == IntOff);` 코드에 의해 어설션에 실패한다.
        currentThread->Sleep();
    } else {
        // 인터럽트를 다시 활성화한다.
        interrupt->SetLevel(prevIntLevel);
    }
}

void MySemaphore::V() {
    // 인터럽트를 비활성화 시킴으로서 V() 함수를 실행하는 동안 컨텍스트 스위칭이 일어나지 않도록 한다.
    IntStatus prevIntLevel = interrupt->SetLevel(IntOff);

    // 세마포어 값을 증가한다.
    this->value++;

    // 만약 현재 세마포어의 획득을 기다리고 있는 쓰레드가 존재하는 경우,
    if (value <= 0) {

        // 큐에서 쓰레드를 가지고 온다.
        Thread *thread = (Thread *) this->queue->Remove();

        // value 값은 세마포어의 실행을 대기하고 있는 쓰레드의 갯수이기 때문에, 무조건 쓰레드를 큐에서 가져올 수 있어야 한다.
        ASSERT(thread != NULL);

        // 방금 꺼낸 쓰레드를 스케줄러에게 ReadyToRun 상태로 변경을 요청한다.
        scheduler->ReadyToRun(thread);
    }

    // 인터럽트를 다시 활성화한다.
    interrupt->SetLevel(prevIntLevel);
}
