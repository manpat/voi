#include "mirrormanager.h"

Mirror::Mirror(s32 l0, s32 l1) : Component(this) {
	layer[0] = l0;
	layer[1] = l1;
}

void Mirror::OnInit() {
	std::cout << "Mirror created (id:" << id << ", layer: " << layer[0] << ", dest. layer: " << layer[1] << ')' << std::endl;
}

void Mirror::OnDestroy() {
	std::cout << "Mirror destroyed (id: " << id << ')' << std::endl;
}