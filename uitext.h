#ifndef UITEXT_H
#define UITEXT_H

#include "uiobject.h"

struct UiText : UiObject {
public:
	UiText();
	~UiText();
private:
	std::string text;
};

#endif//UITEXT_H