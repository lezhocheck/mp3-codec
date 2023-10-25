#include "paginator.h"
#include "CodeScraper.h"


void mp3_codec::Paginator::add(String^ file)
{
    // System::String^ path = System::IO::Directory::GetCurrentDirectory() + "/" + file;
    // if (!System::IO::File::Exists(path)) {
    //     throw std::invalid_argument("Invalid file local path provided");
    // }
    // CodeScraper^ scraper = gcnew CodeScraper(path, start, end);
    ResourceManager^ rm = gcnew ResourceManager("mp3_codec.CodeResource", GetType()->Assembly);
    items->Add(rm->GetString(file));
}

bool mp3_codec::Paginator::hasNext()
{
    if (items->Count == 0) {
        return false;
    }
    return currentPage < items->Count - 1;
}

bool mp3_codec::Paginator::hasPrev()
{
    if (items->Count == 0) {
        return false;
    }
    return currentPage > 0;
}

void mp3_codec::Paginator::nextPage() {
    if (currentPage + 1 < items->Count) {
        currentPage++;
    }
}

System::String^ mp3_codec::Paginator::getName()
{
    return name;
}

void mp3_codec::Paginator::previousPage() {
    if (currentPage > 0) {
        currentPage--;
    }
}

System::String^ mp3_codec::Paginator::displayCurrentPage() {
    if (items->Count == 0) {
        return "";
    }
    return items[currentPage];
}

int mp3_codec::Paginator::getCurrentPage(){
    return currentPage;
}
