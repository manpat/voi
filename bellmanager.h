#ifndef BELLMANAGER_H
#define BELLMANAGER_H

#include "audiogenerator.h"
#include "singleton.h"
#include "common.h"
#include <vector>

struct Bell;

struct BellManager : Singleton<BellManager> {
	std::vector<Bell*> bells;

	void AddBell(Bell*);
	void StopAllBells();
	void CorrectCombination();
};

#endif