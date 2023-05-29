#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Visitor;
class Element {
public:
    virtual void accept(Visitor *visitor) = 0;
};

class ElementA;
class ElementB;
class Visitor {
public:
    virtual void visit(ElementA *elementA) = 0;
    virtual void visit(ElementB *elementB) = 0;
};

class ElementA: public Element {
public:
    void show() { cout << "this is element A" << endl; }
    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};
class ElementB: public Element {
public:
    void show() { cout << "this is element B" << endl; }
    void accept(Visitor *visitor) override {
        visitor->visit(this);
    }
};

class Shower: public Visitor {
public:
    void visit(ElementA *elementA) override { elementA->show(); }
    void visit(ElementB *elementB) override { elementB->show(); }
};

int main() {
    ElementA elementA;
    ElementB elementB;
    Shower shower;
    elementA.accept(&shower);
    elementB.accept(&shower);
    return 0;
}