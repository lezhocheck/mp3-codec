#pragma once

#include <vector>
#include <string>


public ref class Paginator {
private:
    System::Collections::Generic::List<System::String^>^ items;
    int currentPage;
public:
    Paginator() {
        items = gcnew System::Collections::Generic::List<System::String^>();
        currentPage = 0;
    }
    void add(System::String^ file, int start, int end);
    void nextPage();
    void previousPage();
    System::String^ displayCurrentPage();
    int getCurrentPage();
};
