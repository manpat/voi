#include "voi.h"
#include <vector>
#include <fstream>

namespace {
	std::vector<char*> staticallyLoadedFiles;
}

FileData LoadFile(const char* filename, bool nullTerminate) {
	FileData fdata{nullptr, 0};

	if(std::ifstream file{filename, file.binary | file.ate}) {
		fdata.size = file.tellg();
		fdata.data = new char[fdata.size + (nullTerminate?1:0)];
		assert(fdata.data);
		
		file.seekg(0, file.beg);
		file.read((char*)fdata.data, fdata.size);

		if(nullTerminate)
			fdata.data[fdata.size] = 0;
	}

	return fdata;
}

char* LoadFileStatically(const char* filename, bool nullTerminate) {
	char* data = LoadFile(filename, nullTerminate).data;
	if(data)
		staticallyLoadedFiles.emplace_back(data);

	return data;
}

void CleanupStaticallyLoadedFiles() {
	for(auto f: staticallyLoadedFiles) 
		delete[] f;

	staticallyLoadedFiles.clear();
}