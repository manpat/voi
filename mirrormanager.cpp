#include "mirrormanager.h"

void Mirror::OnInit() {
	std::cout << "Mirror(" << id << ") - OnInit fired" << std::endl;
}

void Mirror::OnDestroy() {
	std::cout << "Mirror(" << id << ") - OnDestroy fired" << std::endl;
}