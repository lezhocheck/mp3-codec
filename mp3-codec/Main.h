#pragma once
#include <fstream>
#include <string>
#include <iostream>
#include <msclr/marshal_cppstd.h>
#include "Code.h"
#include "open.h"
#include "winplayer.h"
#include "paginator.h"
#include "mpeg.h" 

namespace mp3_codec {

	using namespace System;
	using namespace System::Globalization;
	using namespace System::Threading;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Resources;
	using namespace System::Drawing;
	using namespace msclr::interop;

	/// <summary>
	/// Summary for Main
	/// </summary>
	public ref class Main : public System::Windows::Forms::Form
	{
	public:
		Main(void)
		{
			InitializeComponent();
			SetupCodeForm();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Main()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::MenuStrip^ menuStrip1;
	protected:
	private: System::Windows::Forms::ToolStripMenuItem^ fileToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ languageToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ englishToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ ukrainianToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ exitToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^ codeToolStripMenuItem;

	private: System::Windows::Forms::Button^ button1;
	private: System::Windows::Forms::TextBox^ textBox1;
	private: System::ComponentModel::BackgroundWorker^ backgroundWorker;
	private: System::Windows::Forms::Button^ button2;
	private: System::Windows::Forms::RichTextBox^ fileInfo;








	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->codeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->languageToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->englishToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ukrainianToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->backgroundWorker = (gcnew System::ComponentModel::BackgroundWorker());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->fileInfo = (gcnew System::Windows::Forms::RichTextBox());
			this->menuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->fileToolStripMenuItem,
					this->codeToolStripMenuItem, this->languageToolStripMenuItem, this->exitToolStripMenuItem
			});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(284, 24);
			this->menuStrip1->TabIndex = 0;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this->fileToolStripMenuItem->Name = L"fileToolStripMenuItem";
			this->fileToolStripMenuItem->Size = System::Drawing::Size(37, 20);
			this->fileToolStripMenuItem->Text = L"File";
			// 
			// codeToolStripMenuItem
			// 
			this->codeToolStripMenuItem->Name = L"codeToolStripMenuItem";
			this->codeToolStripMenuItem->Size = System::Drawing::Size(47, 20);
			this->codeToolStripMenuItem->Text = L"Code";
			this->codeToolStripMenuItem->Click += gcnew System::EventHandler(this, &Main::codeToolStripMenuItem_Click);
			// 
			// languageToolStripMenuItem
			// 
			this->languageToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->englishToolStripMenuItem,
					this->ukrainianToolStripMenuItem
			});
			this->languageToolStripMenuItem->Name = L"languageToolStripMenuItem";
			this->languageToolStripMenuItem->Size = System::Drawing::Size(71, 20);
			this->languageToolStripMenuItem->Text = L"Language";
			// 
			// englishToolStripMenuItem
			// 
			this->englishToolStripMenuItem->Name = L"englishToolStripMenuItem";
			this->englishToolStripMenuItem->Size = System::Drawing::Size(124, 22);
			this->englishToolStripMenuItem->Text = L"English";
			this->englishToolStripMenuItem->Click += gcnew System::EventHandler(this, &Main::englishToolStripMenuItem_Click);
			// 
			// ukrainianToolStripMenuItem
			// 
			this->ukrainianToolStripMenuItem->Name = L"ukrainianToolStripMenuItem";
			this->ukrainianToolStripMenuItem->Size = System::Drawing::Size(124, 22);
			this->ukrainianToolStripMenuItem->Text = L"Ukrainian";
			this->ukrainianToolStripMenuItem->Click += gcnew System::EventHandler(this, &Main::ukrainianToolStripMenuItem_Click);
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(38, 20);
			this->exitToolStripMenuItem->Text = L"Exit";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &Main::exitToolStripMenuItem_Click);
			// 
			// button1
			// 
			this->button1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->button1->Location = System::Drawing::Point(197, 40);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 2;
			this->button1->Text = L"Open";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Main::button1_Click);
			// 
			// textBox1
			// 
			this->textBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox1->Location = System::Drawing::Point(12, 43);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(179, 20);
			this->textBox1->TabIndex = 3;
			// 
			// backgroundWorker
			// 
			this->backgroundWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &Main::backgroundWorker_DoWork);
			this->backgroundWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &Main::backgroundWorker_RunWorkerCompleted);
			// 
			// button2
			// 
			this->button2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Left | System::Windows::Forms::AnchorStyles::Right));
			this->button2->AutoSize = true;
			this->button2->Location = System::Drawing::Point(12, 235);
			this->button2->MinimumSize = System::Drawing::Size(260, 0);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(260, 23);
			this->button2->TabIndex = 4;
			this->button2->Text = L"Play MPEG file";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Main::button2_Click);
			// 
			// fileInfo
			// 
			this->fileInfo->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->fileInfo->Location = System::Drawing::Point(12, 69);
			this->fileInfo->MinimumSize = System::Drawing::Size(260, 4);
			this->fileInfo->Name = L"fileInfo";
			this->fileInfo->ReadOnly = true;
			this->fileInfo->Size = System::Drawing::Size(260, 140);
			this->fileInfo->TabIndex = 5;
			this->fileInfo->Text = L"";
			// 
			// Main
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->fileInfo);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->menuStrip1);
			this->MainMenuStrip = this->menuStrip1;
			this->MaximizeBox = false;
			this->MaximumSize = System::Drawing::Size(300, 300);
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(300, 300);
			this->Name = L"Main";
			this->Text = L"mpeg";
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: 
		System::Void codeToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
			SetupCodeForm();
		}

		void SetupCodeForm() {
			Code^ codeForm = Code::GetInstance();
			codeForm->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &Main::Code_FormClosing);
			codeForm->Show();
			codeToolStripMenuItem->Enabled = false;
		}

		System::Void Code_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e) {
			codeToolStripMenuItem->Enabled = true;
		}
		System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
			Stream^ myStream;
			OpenFileDialog^ dialog = gcnew OpenFileDialog;
			dialog->InitialDirectory = "c:\\";
			dialog->Filter = "MP2 files (*.mp2)|*.mp2";;
			dialog->FilterIndex = 2;
			dialog->RestoreDirectory = true;

			if (dialog->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				textBox1->Text = dialog->FileName;
				Code^ codeForm = Code::GetInstance();
				Paginator^ paginator = gcnew Paginator();
				paginator->add("wav.cpp", 217, 234);
				paginator->add("wav.cpp", 26, 52);
				codeForm->SetPaginator(paginator);

				std::string path = marshal_as<std::string>(textBox1->Text);
				std::ifstream input(path, std::ios::in | std::ios::binary);
				if (!input.is_open()) {
					return;
				}
				if (!IsMpegFile(input)) {
					MessageBox::Show("Not an MPEG file", "Error",
						MessageBoxButtons::OK, MessageBoxIcon::Error);
					return;
				}
				std::ostringstream log;
				MpegRead audioFile(input, log);
				fileInfo->Text = gcnew String(log.str().c_str());
			}
		}
	
		System::Void englishToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
			Thread::CurrentThread->CurrentUICulture = CultureInfo::CreateSpecificCulture("en-US");
			SetupResources();
		}
		System::Void ukrainianToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
			Thread::CurrentThread->CurrentUICulture = CultureInfo::CreateSpecificCulture("uk-UA");
			SetupResources();
		}

		void SetupResources() {
			ResourceManager^ rm = gcnew ResourceManager("mp3_codec.Resources", GetType()->Assembly);
			codeToolStripMenuItem->Text = rm->GetString("code");
			fileToolStripMenuItem->Text = rm->GetString("file");
			languageToolStripMenuItem->Text = rm->GetString("language");
			exitToolStripMenuItem->Text = rm->GetString("exit");
			englishToolStripMenuItem->Text = rm->GetString("english");
			ukrainianToolStripMenuItem->Text = rm->GetString("ukrainian");
			button1->Text = rm->GetString("open");
			button2->Text = rm->GetString("playMpeg");
		}

		
		System::Void exitToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
			Application::Exit();
		}
		
		System::Void backgroundWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
			String^ str = Convert::ToString(e->Argument);
			std::string path = marshal_as<std::string>(str);
			std::ifstream input(path, std::ios::in | std::ios::binary);
			if (!input.is_open()) {
				return;
			}
			std::ostringstream out;
			AudioAbstract* audioFile = new MpegRead(input, out);
			if (audioFile) {
				WinPlayer player(audioFile);

				player.Play();
				delete audioFile;
			}
		}
		System::Void backgroundWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
		
		}
		System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
			if (!backgroundWorker->IsBusy) {
				Code^ codeForm = Code::GetInstance();
				Paginator^ paginator = gcnew Paginator();
				paginator->add("winplayer.cpp", 7, 53);
				paginator->add("winplayer.cpp", 125, 240);
				paginator->add("winplayer.cpp", 55, 100);
				codeForm->SetPaginator(paginator);

				//output file details
				std::ostringstream out;
				std::ostringstream out2;
				std::string path = marshal_as<std::string>(textBox1->Text);
				std::ifstream input(path, std::ios::in | std::ios::binary);
				if (!input.is_open()) {
					return;
				}
				AudioAbstract* audioFile = new MpegRead(input, out2);
				if (audioFile) {
					WinPlayer player(audioFile);
					player.PrintInfo(out);
					fileInfo->Text = gcnew String(out.str().c_str());	
				}
				backgroundWorker->RunWorkerAsync(textBox1->Text);
			}
		}
private: System::Void button3_Click(System::Object^ sender, System::EventArgs^ e) {
}
};
}
