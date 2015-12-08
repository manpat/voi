#ifndef UITEXT_H
#define UITEXT_H

#include "uiobject.h"

struct UiText : UiObject {
public:
	UiText();
	~UiText();

	void Init();

	void SetText(std::string text);
	void SetFont(const std::string& filename);
	void ResizeObjectToText();

	void SetColour(f32 r, f32 g, f32 b, f32 a) override;
	void SetPosition(f32 x, f32 y) override;
	void SetSize(u32 w, u32 h) override;
	void SetVisible(bool visible = true) override;

private:
	std::string text;
};

#endif//UITEXT_H