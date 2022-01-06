#ifndef TIMEOBJ_H 
#define TIMEOBJ_H
class TimeObj {
    int timeCount;
public:

    TimeObj() {
        timeCount = 0;
    }

    int readTime() {
        return this->timeCount;
    }

    void incCounter() {
        this->timeCount++;
    }

};

#endif