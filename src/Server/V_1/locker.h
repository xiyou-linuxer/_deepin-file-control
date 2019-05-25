#ifndef __LOCKER_H_
#define __LOCKER_H_
#include "model.h"
class Sem {
 public:
  Sem() {
    if (sem_init(&sem, 0, 0) != 0) throw exception();
  }
  ~Sem() { sem_destroy(&sem); }
  bool wait() { return sem_wait(&sem) == 0; }
  bool post() { return sem_post(&sem) == 0; }

 private:
  sem_t sem;
};
class Locker {
 public:
  Locker() {
    if (pthread_mutex_init(&mutx, NULL) != 0) throw exception();
  }
  ~Locker() { pthread_mutex_destroy(&mutx); }
  bool lock() { return pthread_mutex_lock(&mutx) == 0; }
  bool unlock() { return pthread_mutex_unlock(&mutx) == 0; }

 private:
  pthread_mutex_t mutx;
};
class Cond {
  Cond() {
    if (pthread_mutex_init(&mutx, NULL) != 0) throw exception();
    if (pthread_cond_init(&cond, NULL) != 0) {
      pthread_mutex_destroy(&mutx);
      throw exception();
    }
  }
  ~Cond() {
    pthread_mutex_destroy(&mutx);
    pthread_cond_destroy(&cond);
  }
  bool wait() {
    int ret = 0;
    pthread_mutex_lock(&mutx);
    ret = pthread_cond_wait(&cond, &mutx);
    pthread_mutex_unlock(&mutx);
    return ret == 0;
  }
  bool signal() { return pthread_cond_signal(&cond) == 0; }

 private:
  pthread_mutex_t mutx;
  pthread_cond_t cond;
};
#endif