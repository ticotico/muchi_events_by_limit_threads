/*
1050608
Much events by Limit thread.
In this program, there are 10 events.
To prevent overfull thread, this program limits up to 4 threads.

useage:
1. Copy Class Event and Class multiEventThread andexecute_event function
2. Add new events than set by Event::setEvent
3. Add Events to  multiEventThread object by add_event.
4. Start multi threads by multiEventThread::execute

*/

#include <process.h>
#include <cassert>
#include <list>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

void execute_event(void* pEvent);

class Event {
friend void execute_event(void*);
private:
	bool finish;
	void (*funcAddress)(void*);
	unsigned int stack_num;
	void* listptr;
	
	
public:
	Event() : finish(false), funcAddress(NULL), stack_num(0), listptr(NULL) {}
	
	void setEvent(void (*funAddress)(void*), unsigned int stack, void* iptr) {
		funcAddress = funAddress;
		stack_num = stack;
		listptr = iptr;
		assert(funcAddress != NULL);
	}
	void execute() {
		_beginthread(execute_event, stack_num, this);
	}
	bool isFinish() {
		return finish;
	}
};

void execute_event(void* pEvent) {
	Event* tmp = (Event*) pEvent;
	tmp->funcAddress(tmp->listptr);
	tmp->finish = true;
}

class multiEventThread {
private:
	int max_parallel_thread;
	typedef std::list<Event> VEvent;
	VEvent undoEvent;
	VEvent doingEvent;
	VEvent finishEvent;
public:
	void init(int max_thread) {
		max_parallel_thread = max_thread;
	}
	void add_event(const Event& event) {
		undoEvent.push_back(event);
	}
	void execute() {
		while (undoEvent.size() > 0 || doingEvent.size() > 0) {
			//如果使用執行續未滿 
			while (doingEvent.size() < max_parallel_thread &&
					undoEvent.size() > 0) {
				//從未作事件取出一個來做
				Event tmp;
				tmp = undoEvent.front() ;
				undoEvent.pop_front();
				doingEvent.push_back(tmp);
				doingEvent.back().execute();
			}
			//檢查正在做的 是否完成
			VEvent::iterator iter = doingEvent.begin();
			while (iter != doingEvent.end()) {
				if (iter->isFinish()) {
					Event tmp;
					tmp = *iter;
					doingEvent.erase(iter);
					finishEvent.push_back(tmp);
					iter = doingEvent.begin();
					continue;
				}
				++iter;
			}
			//隔一段時間再檢查 
			//Sleep(100);
		}
	}
	bool allEventFinish() {
		return (undoEvent.size() + doingEvent.size() == 0);
	}
	void PRINT_DEBUG_DATA(int line) {
		std::cout << line << std::endl;
		std::cout << "undoEvent.size() = " << undoEvent.size() <<std::endl;
		std::cout << "doingEvent.size() = " << doingEvent.size() << std::endl;
		std::cout << std::endl;
	}
};

CRITICAL_SECTION CriticalSection;

enum {
	event_max_time = 1000
};

class toDofuncIndex {
public:
	int runtime;
	int i;
};

void toDofunc(void* ptr) {
	toDofuncIndex *index = (toDofuncIndex*) ptr;
	assert(index != NULL);
	assert(index->runtime > 0);
	
	int runtime = index->runtime;
	int i = index->i;
	Sleep(runtime);
	EnterCriticalSection(&CriticalSection);
	std::cout << "task " << i << " finished" << std::endl;
	LeaveCriticalSection(&CriticalSection);
}


int main() {
	if (!InitializeCriticalSectionAndSpinCount(&CriticalSection, 0x400))
		return 0;
	
	srand(time(NULL));
	multiEventThread met;
	met.init(4);
	//事件參數初始化
	toDofuncIndex funindex[10]; 
	for (int i = 0; i < 10; i++) {
		funindex[i].runtime = rand() % event_max_time;
		funindex[i].i = i;
	}
	//把事件加入到執行緒處理類別中 
	for (int i = 0; i < 10; i++) {
		Event tmp;
		tmp.setEvent(toDofunc, 0, &(funindex[i]));
		met.add_event(tmp);
	}
	//開始處理多執行續 
	met.execute();
	DeleteCriticalSection(&CriticalSection);
	return 0;
}
