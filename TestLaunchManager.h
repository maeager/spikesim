// TestManager.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TESTMANAGER_H
#define TESTMANAGER_H


class TestUnit
{
public:
    static void launch_tests() {
        file_.open()
    }
private:
    static std::ofstream file_;
};


#endif // !defined TESTMANAGER_H