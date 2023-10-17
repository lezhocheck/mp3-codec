#include <iostream>
#include <fstream>
#include <string>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;

public ref class CodeScraper
{
private:
    String^ filePath;
    int lineStart;
    int lineEnd;

public:
    CodeScraper(String^ path, int startLine, int endLine);
    List<String^>^ Process();
};
