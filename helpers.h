#ifndef HELPERS_H
#define HELPERS_H

#include <map>

template<typename K, typename V>
V findin(const std::map<K,V>& m, K k, V dv){
	auto it = m.find(k);
	if(it == m.end()) return dv;

	return it->second;
}

#endif