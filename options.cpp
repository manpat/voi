#include "voi.h"
#include "ext/stb.h"

#include <algorithm>
#include <ctype.h>
#include <vector>

struct Option {
	enum Type {
		TypeNone,
		TypeString,
		TypeBool,
		TypeInt,
		TypeFloat,
	};

	char name[64];
	char value[64];
	u8 type;
	// TODO: Add description field

	union {
		bool boolValue;
		s64 intValue;
		f64 floatValue;
	};

	static Option String(const char* _name, const char* b) {
		Option o;
		o.type = TypeString;
		snprintf(o.name, 64, "%s", _name);
		snprintf(o.value, 64, "%s", b);
		return o;
	}

	static Option Bool(const char* _name, bool b) {
		Option o;
		o.type = TypeBool;
		o.boolValue = b;
		snprintf(o.name, 64, "%s", _name);
		snprintf(o.value, 64, "%s", b?"true":"false");
		return o;
	}
	static Option Int(const char* _name, s64 b) {
		Option o;
		o.type = TypeInt;
		o.intValue = b;
		snprintf(o.name, 64, "%s", _name);
		snprintf(o.value, 64, "%ld", b);
		return o;
	}
	static Option Float(const char* _name, f64 b) {
		Option o;
		o.type = TypeFloat;
		o.floatValue = b;
		snprintf(o.name, 64, "%s", _name);
		snprintf(o.value, 64, "%f", b);
		return o;
	}

	operator bool() {
		return type != TypeNone;
	}
};

namespace {
	static const char* cfgName = "voi.cfg";
	static const char* cfgHeaderStr = 
		"# Voi configuration\n"
		"# Modify at your own risk\n";
		
	static Option defaultOptions[] = {
		Option::Int  ("window.width", 800),
		Option::Int  ("window.height", 600),
		Option::Bool ("window.fullscreen", false),
		Option       (),
		Option::Float("graphics.fov", 60.f),
		Option::Float("graphics.cursorsize", 1.5f), // Percentage of screen vertically
		Option::Bool ("graphics.multisample", false),
		Option       (),
		Option::Float("input.mousespeed", 1.f),
		Option::Float("input.smoothing_coeff", 0.8f),
	};

	std::vector<Option> options;
	std::vector<Option> cloptions;
}

static Option ParseOption(char*);
static void WriteAndSetDefaultOptions();

void ParseCLOptions(s32 ac, char** av) {
	if(ac <= 1) return;
	cloptions.reserve(ac-1);

	for(s32 i = 1; i < ac; i++) {
		auto o = ParseOption(av[i]);
		Log("Opt: %s = %s\t\ttype: %hhu\n", o.name, o.value, o.type);
		if(o) cloptions.push_back(o);
	}
}

void LoadOptions() {
	if(!stb_fexists(cfgName)) {
		WriteAndSetDefaultOptions();
		return;
	}

	s32 lineCount = 0;
	auto lines = stb_stringfile_trimmed(cfgName, &lineCount, '#');
	if(!lines){
		LogError("Warning! Unable to read %s\n", cfgName);
		return;
	}

	for(s32 i = 0; i < lineCount; i++) {
		auto line = lines[i];
		if(auto c = strchr(line, '#')){
			*c = '\0';
		}
		
		auto o = ParseOption(line);
		if(o) options.push_back(o);

		Log("Opt: %s = %s\t\ttype: %hhu\n", o.name, o.value, o.type);
	}

	free(lines);
}

Option* GetOption(const char* oname) {
	auto comp = [oname] (const Option& o) {
		return !strcmp(oname, o.name);
	};

	auto it = std::find_if(cloptions.begin(), cloptions.end(), comp);
	if(it != cloptions.end()) {
		return &*it;
	}

	it = std::find_if(options.begin(), options.end(), comp);
	if(it != options.end()) {
		return &*it;
	}

	auto it2 = std::find_if(std::begin(defaultOptions), std::end(defaultOptions), comp);
	if(it2 != std::end(defaultOptions)){
		return it2;
	}

	return nullptr;
}

bool GetBoolOption(const char* oname) {
	auto opt = GetOption(oname);

	if(opt) {
		switch(opt->type) {
			case Option::TypeBool: return opt->boolValue;
			case Option::TypeInt: return opt->intValue != 0;
			case Option::TypeFloat: return opt->floatValue != 0.0;
			case Option::TypeString: {
				LogError("Warning! Expected option \"%s\" to be a bool, got a string\n", oname);
				return 0;
			}
		}
	}

	return false;
}

s64 GetIntOption(const char* oname) {
	auto opt = GetOption(oname);

	if(opt) {
		switch(opt->type) {
			case Option::TypeInt: return opt->intValue;
			case Option::TypeBool: return (s64)opt->boolValue;
			case Option::TypeFloat: return (s64)opt->floatValue;
			case Option::TypeString: {
				LogError("Warning! Expected option \"%s\" to be an int, got a string\n", oname);
				return 0;
			}
		}
	}

	return 0;
}

f64 GetFloatOption(const char* oname) {
	auto opt = GetOption(oname);

	if(opt) {
		switch(opt->type) {
			case Option::TypeBool: return (f64)opt->boolValue;
			case Option::TypeInt: return (f64)opt->intValue;
			case Option::TypeFloat: return opt->floatValue;
			case Option::TypeString: {
				LogError("Warning! Expected option \"%s\" to be a float, got a string\n", oname);
				return 0;
			}
		}
	}

	return 0.0;
}

const char* GetStringOption(const char* oname) {
	if(auto opt = GetOption(oname))
		return opt->value;
	return "";
}

static void WriteAndSetDefaultOptions() {
	options.clear();
	options.insert(options.begin(), std::begin(defaultOptions), std::end(defaultOptions));

	constexpr u32 bufferSize = 1024<<4;
	char buffer[bufferSize] {0};

	s32 remaining = bufferSize;
	char* it = buffer;

	s32 written = std::snprintf(it, remaining, "%s\n", cfgHeaderStr);
	remaining -= written;
	it += written;

	for(auto& opt : defaultOptions) {
		if(remaining <= 0) {
			LogError("Warning! Ran out of space while writing default cfg! Maybe allocate some more space?\n");
			break;
		}

		if(opt.type == Option::TypeNone) {
			*it++ = '\n'; 
			remaining--;
			continue;
		}

		s32 written = std::snprintf(it, remaining, "%-20s = %s\n", opt.name, opt.value);
		remaining -= written;
		it += written;
	}

	if(auto f = fopen(cfgName, "wb")) {
		fwrite(buffer, strlen(buffer), 1, f);
		fclose(f);
	}else{
		LogError("Warning! Unable to open %s to write defaults\n", cfgName);
	}
}

static Option ParseOption(char* s) {
	char* equals = strchr(s, '=');
	if(!equals) return Option();
	*equals = '\0';
		
	auto name = stb_trimwhite(s);
	auto value = stb_trimwhite(equals+1);
	u32 valueLength = strlen(value);
	if(strlen(name) == 0 || valueLength == 0) Option();

	if(!strcmp("true", value)) {
		return Option::Bool(name, true);
	}else if(!strcmp("false", value)) {
		return Option::Bool(name, false);
	}

	bool isFloat = false;
	bool isNumber = true;

	for(u32 i = 0; i < valueLength; i++) {
		auto c = value[i];
		if(isdigit(c)) continue;

		// Localisation, yo
		if(c == ',') c = value[i] = '.';
		if(c == '.') {
			if(!isFloat) {
				isFloat = true;
				continue;
			}
		}
		isNumber = isFloat = false;
		break;
	}

	if(isFloat) {
		return Option::Float(name, std::atof(value));
	}else if(isNumber) {
		return Option::Int(name, std::atoll(value));
	}
	return Option::String(name, value);
}