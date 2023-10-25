#include "paginator.h"
#include "CodeScraper.h"


void Paginator::add(String^ file, int start, int end)
{
    System::String^ path = System::IO::Directory::GetCurrentDirectory() + "/" + file;
    if (!System::IO::File::Exists(path)) {
        throw std::invalid_argument("Invalid file local path provided");
    }
    CodeScraper^ scraper = gcnew CodeScraper(path, start, end);
    items->Add(String::Join("\n", scraper->Process()));
}

bool Paginator::hasNext()
{
    if (items->Count == 0) {
        return false;
    }
    return currentPage < items->Count - 1;
}

bool Paginator::hasPrev()
{
    if (items->Count == 0) {
        return false;
    }
    return currentPage > 0;
}

void Paginator::nextPage() {
    if (currentPage + 1 < items->Count) {
        currentPage++;
    }
}

System::String^ Paginator::getName()
{
    return name;
}

void Paginator::previousPage() {
    if (currentPage > 0) {
        currentPage--;
    }
}

System::String^ Paginator::displayCurrentPage() {
    if (items->Count == 0) {
        return "";
    }
    return items[currentPage];
}

int Paginator::getCurrentPage(){
    return currentPage;
}
