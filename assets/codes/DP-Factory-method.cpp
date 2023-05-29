#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Shoe {
public:
    virtual void show() = 0;
    virtual ~Shoe() {}
};

class Factory {
public:
    void show() {
        Shoe *shoe = makeShoes();
        shoe->show();
        delete shoe;
    }
    virtual Shoe *makeShoes() = 0;
};

class LiNing: public Shoe {
public:
    void show() override { cout << "There is LiNing~" << endl; }
};
class Nike: public Shoe {
public:
    void show() override { cout << "There is Nike~" << endl; }
};

class LiNingFactory: public Factory {
public:
    Shoe * makeShoes() override { return new LiNing(); }
};
class NikeFactory: public Factory {
public:
    Shoe * makeShoes() override { return new Nike(); }
};

int main() {
    LiNingFactory liNingFactory;
    NikeFactory nikeFactory;
    liNingFactory.show();
    nikeFactory.show();
    return 0;
}