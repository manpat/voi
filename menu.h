#ifndef MENU_H
#define MENU_H

class App;

class Menu {
public:
	static Menu& Inst() {
		static Menu inst;	
		return inst;
	}

	void Init();
	void Uninit();
	void Update();

private:
	Menu() {}

};

#endif // MENU_H