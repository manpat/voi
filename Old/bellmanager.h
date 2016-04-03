#ifndef BELLMANAGER_H
#define BELLMANAGER_H

#include "audiogenerator.h"
#include "singleton.h"
#include "common.h"
#include <vector>
#include <map>

struct Bell;

struct BellManager : Singleton<BellManager> {
	std::map<std::string, std::vector<Bell*>> bells;

	void AddBell(const std::string& target, Bell*);
	void StopAllBells(const std::string& target);
	void CorrectCombination(const std::string& target);
};

#endif