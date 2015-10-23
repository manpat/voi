#ifndef BELL_H
#define BELL_H

#include "component.h"

struct BellAudioGenerator;

struct Bell : Component {
	Entity* target = nullptr;
	std::shared_ptr<BellAudioGenerator> bellGen;
	std::string targetName;
	u32 bellNumber;

	Bell(const std::string& tn, u32 num) : Component(this), targetName(tn), bellNumber(num) {}

	void OnAwake() override;
	void OnMessage(const std::string&, const OpaqueType&) override;
};

#endif