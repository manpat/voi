#include "component.h"

bool Component::SameType(Component* c) const {
	return c && (c->typeHash == typeHash);
}
