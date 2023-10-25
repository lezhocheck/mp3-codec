#pragma once
#include <filesystem>
#include "CodeScraper.h"
#include "paginator.h"

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
	private: 
		Paginator^ paginator;
	public:
		Code(void)
		{
			InitializeComponent();
			SetPaginator(gcnew Paginator("code"));
		}

		static Code^ GetInstance()
		{
			if (instance == nullptr || instance->IsDisposed)
			{
				instance = gcnew Code();
			}
			return instance;
		}

		void SetPaginator(Paginator^ paginator) {
			this->paginator = paginator;
			this->Text = paginator->getName();
			textBox->Text = paginator->displayCurrentPage();
			nextButton->Enabled = paginator->hasNext();
			prevButton->Enabled = paginator->hasPrev();
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
	public: System::Windows::Forms::Button^ nextButton;
	public: System::Windows::Forms::Button^ prevButton;

		   static Code^ instance = nullptr;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->textBox = (gcnew System::Windows::Forms::RichTextBox());
			this->nextButton = (gcnew System::Windows::Forms::Button());
			this->prevButton = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// textBox
			// 
			this->textBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->textBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->textBox->Location = System::Drawing::Point(0, 0);
			this->textBox->Margin = System::Windows::Forms::Padding(0);
			this->textBox->Name = L"textBox";
			this->textBox->ReadOnly = true;
			this->textBox->Size = System::Drawing::Size(285, 213);
			this->textBox->TabIndex = 0;
			this->textBox->Text = L"";
			// 
			// nextButton
			// 
			this->nextButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->nextButton->Location = System::Drawing::Point(197, 226);
			this->nextButton->Name = L"nextButton";
			this->nextButton->Size = System::Drawing::Size(75, 23);
			this->nextButton->TabIndex = 1;
			this->nextButton->Text = L"Next";
			this->nextButton->UseVisualStyleBackColor = true;
			this->nextButton->Click += gcnew System::EventHandler(this, &Code::nextButton_Click);
			// 
			// prevButton
			// 
			this->prevButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->prevButton->Location = System::Drawing::Point(116, 226);
			this->prevButton->Name = L"prevButton";
			this->prevButton->Size = System::Drawing::Size(75, 23);
			this->prevButton->TabIndex = 2;
			this->prevButton->Text = L"Previous";
			this->prevButton->UseVisualStyleBackColor = true;
			this->prevButton->Click += gcnew System::EventHandler(this, &Code::prevButton_Click);
			// 
			// Code
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->prevButton);
			this->Controls->Add(this->nextButton);
			this->Controls->Add(this->textBox);
			this->Name = L"Code";
			this->Text = L"code";
			this->ResumeLayout(false);

		}

#pragma endregion
	System::Void nextButton_Click(System::Object^ sender, System::EventArgs^ e) {
		paginator->nextPage();
		textBox->Text = paginator->displayCurrentPage();
		nextButton->Enabled = paginator->hasNext();
		prevButton->Enabled = paginator->hasPrev();
	}
	System::Void prevButton_Click(System::Object^ sender, System::EventArgs^ e) {
		paginator->previousPage();
		textBox->Text = paginator->displayCurrentPage();
		nextButton->Enabled = paginator->hasNext();
		prevButton->Enabled = paginator->hasPrev();
	}
};
}
