#pragma once
#include "CodeScraper.h"


namespace mp3_codec {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Text::RegularExpressions;

	/// <summary>
	/// Summary for Code
	/// </summary>
	public ref class Code : public System::Windows::Forms::Form
	{
	public:
		Code(void)
		{
			InitializeComponent();

			System::String^ path = "D:\\mp3-codec\\mp3-codec\\mp3-codec\\CodeScraper.cpp";
			CodeScraper^ scraper = gcnew CodeScraper(path, 18, 999);
			textBox->Lines = scraper->Process()->ToArray();
		}

		static Code^ GetInstance()
		{
			if (instance == nullptr || instance->IsDisposed)
			{
				instance = gcnew Code();
			}
			return instance;
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Code()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
	private: System::Windows::Forms::RichTextBox^ textBox;

		   static Code^ instance = nullptr;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->textBox = gcnew System::Windows::Forms::RichTextBox();
			this->SuspendLayout();
			// 
			// textBox
			// 
			this->textBox->Dock = System::Windows::Forms::DockStyle::Fill;
			this->textBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->textBox->Location = System::Drawing::Point(0, 0);
			this->textBox->Name = L"textBox";
			this->textBox->ReadOnly = true;
			this->textBox->Size = System::Drawing::Size(284, 261);
			this->textBox->TabIndex = 0;
			this->textBox->Text = L"";
			// 
			// Code
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->textBox);
			this->Name = L"Code";
			this->Text = L"code";
			this->ResumeLayout(false);

		}

#pragma endregion
	};
}
