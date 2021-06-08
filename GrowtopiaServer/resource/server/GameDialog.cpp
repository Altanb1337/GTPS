#define FMT_HEADER_ONLY
#include "../../Server.hpp"
#include "../../lib/fmt/format.h"

using namespace std;

Dialog& Dialog::addCustom(string format) {
	this->str_ += "\n" + format;
	return *this;
}
Dialog& Dialog::addLabel(DialogInfo size, unsigned short itemId, string label) {
	string labelSize = size == DialogInfo::BIG ? "big" : "small";
	this->str_ += fmt::format("\nadd_label_with_icon|{}|{}|left|{}|", size, label, itemId);
	return *this;
}
Dialog& Dialog::addLabel(DialogInfo size, string label) {
	string labelSize = size == DialogInfo::BIG ? "big" : "small";
	this->str_ += fmt::format("\nadd_label|{}|{}|", labelSize, label);
	return *this;
}
Dialog& Dialog::addTextbox(string text) {
	this->str_ += fmt::format("\nadd_textbox|{}|", text);
	return *this;
}
Dialog& Dialog::addSpacer(DialogInfo size) {
	this->str_ += fmt::format("\nadd_spacer|{}|", (size == DialogInfo::BIG ? "big" : "small"));
	return* this;
}
Dialog& Dialog::addInput(bool isPassword, int length, string inputName, string label, string defaultInput) {
	this->str_ += fmt::format("\nadd_text_input{}{}|{}|{}|{}|", (isPassword ? "_password|" : "|"), inputName, label, defaultInput, length);
	return *this;
}
Dialog& Dialog::endDialog(string dialogName, string cancelButton, string acceptButton) {
	this->str_ += fmt::format("\nend_dialog|{}|{}|{}", dialogName, cancelButton, acceptButton);
	return *this;
}