#include "Main.h"

using namespace System;
using namespace System::Globalization;
using namespace System::Threading;
using namespace System::Windows::Forms;


[STAThread]
int main()
{

	Thread::CurrentThread->CurrentUICulture = CultureInfo::CreateSpecificCulture("en-US");
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	Application::Run(gcnew mp3_codec::Main());
	return 0;
}