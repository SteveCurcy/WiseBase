#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Shoe {
public:
    virtual void show() = 0;
    virtual ~Shoe() {}
};

class Clothes {
public:
    virtual void show() = 0;
    virtual ~Clothes() {}
};

class Factory {
public:
    void show() {
        Shoe *shoe = makeShoes();
        shoe->show();
        Clothes *clothes = makeClothes();
        clothes->show();
        delete shoe;
        delete clothes;
    }
    virtual Shoe *makeShoes() = 0;
    virtual Clothes *makeClothes() = 0;
};

class LiNingShoes: public Shoe {
public:
    void show() override { cout << "There is LiNing Shoes~" << endl; }
};
class NikeShoes: public Shoe {
public:
    void show() override { cout << "There is Nike Shoes~" << endl; }
};
class LiNingClothes: public Clothes {
public:
    void show() override { cout << "There is LiNing Clothes~" << endl; }
};
class NikeClothes: public Clothes {
public:
    void show() override { cout << "There is Nike Clothes~" << endl; }
};

class LiNingFactory: public Factory {
public:
    Shoe * makeShoes() override { return new LiNingShoes(); }
    Clothes * makeClothes() override { return new LiNingClothes(); }
};
class NikeFactory: public Factory {
public:
    Shoe * makeShoes() override { return new NikeShoes(); }
    Clothes * makeClothes() override { return new NikeClothes(); }
};

int main() {
    LiNingFactory liNingFactory;
    NikeFactory nikeFactory;
    liNingFactory.show();
    nikeFactory.show();
    return 0;
}