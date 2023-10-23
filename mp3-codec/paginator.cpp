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

void Paginator::nextPage() {
    if (currentPage + 1 < items->Count) {
        currentPage++;
    }
}

void Paginator::previousPage() {
    if (currentPage > 0) {
        currentPage--;
    }
}

System::String^ Paginator::displayCurrentPage() {
    return items[currentPage];
}

int Paginator::getCurrentPage(){
    return currentPage;
}
