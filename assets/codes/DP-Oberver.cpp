#include <iostream>
#include <vector>
#include <string>

using namespace std;

class EventListener {
public:
    virtual void update(const string& filename) = 0;
};

class EventManager {
    vector<EventListener *> listeners;
public:
    void subscribe(EventListener *listener) {
        listeners.push_back(listener);
    }
    void unsubscribe(EventListener *listener) {
        for (auto l = listeners.begin(); l != listeners.end(); l++) {
            if (*l == listener) {
                listeners.erase(l);
                break;
            }
        }
    }
    void notify(const string& data) {
        for (auto *l: listeners) {
            l->update(data);
        }
    }
};

class EmailListener: public EventListener {
public:
    void update(const string &filename) override {
        cout << filename << " has been received..." << endl;
    }
};

int main() {
    EmailListener el;
    EventManager em;
    em.subscribe(&el);
    em.notify("promoteAD");
    em.unsubscribe(&el);
    em.notify("iphoneAD");
    return 0;
}