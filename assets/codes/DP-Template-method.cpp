#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Reader {
protected:
    void openFile() { cout << "open file..." << endl; }
    void closeFile() { cout << "close file..." << endl; }
    virtual void analyse() = 0;
public:
    void read() {
        openFile();
        analyse();
        closeFile();
    }
};

class DocReader: public Reader {
protected:
    void analyse() override { cout << "analyse doc..." << endl; }
};
class PDFReader: public Reader {
protected:
    void analyse() override { cout << "analyse pdf..." << endl; }
};

int main() {
    Reader *reader;
    DocReader docReader;
    PDFReader pdfReader;
    reader = &docReader;
    reader->read();
    reader = &pdfReader;
    reader->read();
    return 0;
}