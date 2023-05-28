#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Strategy {
public:
    virtual void plan() = 0;
};

class Navigator {
    Strategy *strategy;
public:
    void setStrategy(Strategy *s) { strategy = s; }
    void plan() { if (strategy) strategy->plan(); }
};

class Walk: public Strategy {
public:
    void plan() override { cout << "A -> B -> C -> D" << endl; }
};
class Drive: public Strategy {
public:
    void plan() override { cout << "A -> E -> F -> D" << endl; }
};

int main() {
    Navigator navigator;
    Walk walk;
    Drive drive;
    navigator.setStrategy(&walk);
    navigator.plan();
    navigator.setStrategy(&drive);
    navigator.plan();
    return 0;
}