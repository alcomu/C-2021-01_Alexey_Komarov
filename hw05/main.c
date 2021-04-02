#include "mylog.h"

int i = 1;
int var = 0;


void func2(MyLog *logger) {
    ml_write((* logger), LL_INFO, "%d record", i++);
    ml_write((* logger), LL_ERROR, "%d record", i++);
}

void func1(MyLog *logger) {
    ml_write((* logger), LL_INFO, "%d record", i++);

    func2(logger);
}

int main() {

    MyLog logger;

    ml_init(&logger, "ml_out.log");

    ml_write(logger, LL_DEBUG, "%d record", i++);
    ml_write(logger, LL_INFO, "%d record", i++);
    ml_write(logger, LL_WARNING, "%d record", i++);
    // ml_write(logger, LL_ERROR, "%d record", i++);
    // ml_fatal_write(logger, var, "%d record", i++);    
    func1(&logger);

    ml_final(&logger);

    return 0;
}