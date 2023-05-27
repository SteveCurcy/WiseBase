#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Originator;
class Memento {
    int state = 0;
    friend class Originator;
public:
    Memento(int state): state(state) {}
};

class Originator {
    int state;
public:
    Memento *save() { return new Memento(state); }
    void restore(Memento *memento) { state = memento->state };
    void up() { state++; }
    void show() { cout << state << endl; }
};

class Caretaker {
    Originator *originator;
    vector<Memento *> history;
public:
    explicit Caretaker(Originator *o): originator(o) {}
    ~Caretaker() {
        for (auto *m: history) {
            delete m;
        }
    }
    void doSomething() {
        history.emplace_back(originator->save());
        originator->up();
    }
    void undo() {
        originator->restore(history.back());
        delete history.back();
        history.pop_back();
    }
};

int main() {
    Originator *originator = new Originator();
    Caretaker caretaker(originator);

    caretaker.doSomething();
    caretaker.doSomething();
    originator->show();
    caretaker.undo();
    originator->show();

    delete originator;
    return 0;
}