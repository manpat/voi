#include "voi.h"
#include <cstdarg>

namespace {
	FILE* logfile = nullptr;
}

bool InitLog() {
	logfile = std::fopen("voi.log", "wb");
	if(!logfile) return false;

	std::atexit([]{
		std::fclose(logfile);
	});

	return true;
}

void Log(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	
	va_list aq;
	va_copy(aq, ap);
	vfprintf(logfile, fmt, aq);
	va_end(aq);

	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

void LogError(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	
	va_list aq;
	va_copy(aq, ap);
	vfprintf(logfile, fmt, aq);
	va_end(aq);

	vfprintf(stderr, fmt, ap);
	va_end(ap);

}

