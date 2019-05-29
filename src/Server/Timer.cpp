#include "Timer.h"

void Time_Heap::add_timer(Heap_Timer *timer) {
  if (!timer) return;
  if (cur_size >= capacity) Resize();
  int hole = cur_size++;
  int parent = 0;
  for (; hole > 0; hole = parent) {
    parent = (hole - 1) / 2;
    if (array[parent]->expire <= timer->expire) break;
    array[hole] = array[parent];
  }
  array[hole] = timer;
}
void Time_Heap::del_timer(Heap_Timer *timer) {
  if (!timer) return;
  timer->call_back = NULL;
}
Heap_Timer *Time_Heap::top() const {
  if (empty()) return NULL;
  return array[0];
}
void Time_Heap::pop_timer() {
  if (empty()) return;
  if (array[0]) {
    //更改堆首元素内容
    if (Org_num == 0) Org_size = cur_size;
    Org_num++;
    array[0] = array[--cur_size];
    Heap_down(0);
  }
}
class Socket;
extern int efd;
void Time_Heap::tick(File_Opt &s, struct basic *b) {
  Heap_Timer *tmp = array[0];
  time_t cur = time(NULL);
  while (!empty()) {
    if (!tmp) break;
    if (tmp->expire > cur) break;
    if (array[0]->call_back) {
      array[0]->call_back(array[0]->user_data, efd, b, s);
    }
    pop_timer();
    tmp = array[0];
  }
}
void Time_Heap::Heap_down(int hole) {
  Heap_Timer *temp = array[hole];
  int child = 0;
  for (; ((hole * 2 + 1) <= (cur_size - 1)); hole = child) {
    child = hole * 2 + 1;
    if (child < (cur_size - 1) &&
        (array[child + 1]->expire < array[child]->expire)) {
      ++child;
    }
    if (array[child]->expire < temp->expire)
      array[hole] = array[child];
    else
      break;
  }
  array[hole] = temp;
}
void Time_Heap::Resize() {
  Heap_Timer **temp = new Heap_Timer *[2 * capacity];
  for (int i = 0; i < 2 * capacity; i++) {
    temp[i] = NULL;
  }
  if (!temp) {
    throw std::exception();
  }
  capacity = 2 * capacity;
  for (int i = 0; i < cur_size; i++) temp[i] = array[i];
  delete[] array;
  array = temp;
}