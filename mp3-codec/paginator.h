#pragma once

#include <vector>
#include <string>


public ref class Paginator {
private:
    System::Collections::Generic::List<System::String^>^ items;
    int currentPage;
    System::String^ name;
public:
    Paginator(System::String^ name) {
        items = gcnew System::Collections::Generic::List<System::String^>();
        currentPage = 0;
        this->name = name;
    }
    void add(System::String^ file, int start, int end);
    bool hasNext();
    bool hasPrev();
    void nextPage();
    System::String^ getName();
    void previousPage();
    System::String^ displayCurrentPage();
    int getCurrentPage();
};
