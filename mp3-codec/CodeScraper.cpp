#include "CodeScraper.h"


CodeScraper::CodeScraper(String^ path, int startLine, int endLine)
{
    if (startLine >= endLine) {
        throw gcnew Exception("Invalid line range");
    }
    if (!File::Exists(path))
    {
        throw gcnew Exception("File does not exist");
    }
    filePath = path;
    lineStart = startLine;
    lineEnd = endLine;
}

List<String^>^ CodeScraper::Process()
{
    List<String^>^ codeLines = gcnew List<String^>();

    StreamReader^ fileStream = gcnew StreamReader(filePath);
    String^ line;
    int currentLine = 0;

    while ((line = fileStream->ReadLine()) != nullptr)
    {
        currentLine++;

        if (currentLine >= lineStart && currentLine <= lineEnd)
        {
            codeLines->Add(line);
        }
        if (currentLine > lineEnd)
        {
            break;
        }
    }
    fileStream->Close();
    return codeLines;
}
