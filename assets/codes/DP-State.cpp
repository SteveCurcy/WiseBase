#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Document;
class State {
protected:
    Document *document;
public:
    void setDocument(Document *doc) {
        document = doc;
    }
    virtual void commit() = 0;
    virtual void publish() = 0;
};

class Document {
    State *state;
public:
    void setState(State *s) {
        if (state) delete state;
        state = s;
        s->setDocument(this);
    }
    ~Document() {
        if (state) delete state;
    }
    void commit() { state->commit(); }
    void publish() { state->publish(); }
};

class Published: public State {
public:
    void commit() override { cout << "Has been published!" << endl; }
    void publish() override { cout << "Has been published!" << endl; }
};

class Moderation: public State {
public:
    void commit() override { cout << "Has been committed!" << endl; }
    void publish() override {
        document->setState(new Published());
    }
};

class Draft: public State {
    friend class Moderation;
public:
    void publish() override { cout << "Hasn't been committed!" << endl; }
    void commit() override {
        document->setState(new Moderation());
    }
};

int main() {
    State *state = new Draft();
    Document *doc = new Document();
    doc->setState(state);
    doc->publish();
    doc->commit();
    doc->commit();
    doc->publish();
    doc->commit();
    doc->publish();
    return 0;
}