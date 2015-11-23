#ifndef UITEXT_H
#define UITEXT_H

#include "uiobject.h"

struct UiText : UiObject {
public:
	UiText();
	~UiText();

	void Init();

	void SetColour(f32 r, f32 g, f32 b, f32 a);
	void SetText(std::string text);
	void SetFont(const std::string& filename);
	void ResizeObjectToText();
	void SetPosition(f32 x, f32 y);
	void SetSize(u32 w, u32 h);
	void SetVisible(bool visible = true);

private:
	std::string text;
};

#endif//UITEXT_H