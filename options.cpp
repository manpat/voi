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
		Option       (),
		Option::Float("input.mousespeed", 1.f),
	};

	std::vector<Option> options;

	const char* levelName;
}

void ParseCLOptions(s32 ac, const char** av) {
	if(ac <= 1) return;

	for(s32 i = 1; i < ac; i++) {
		auto s = av[i];
		printf("Opt: %s\n", s);

		if(s[0] != '-') {
			levelName = s;
		}else{
			// TODO
		}
	}
}

static void WriteAndSetDefaultOptions();

void LoadOptions() {
	if(!stb_fexists(cfgName)) {
		WriteAndSetDefaultOptions();
		return;
	}

	s32 lineCount = 0;
	auto lines = stb_stringfile_trimmed(cfgName, &lineCount, '#');
	if(!lines){
		printf("Warning! Unable to read %s\n", cfgName);
		return;
	}

	for(s32 i = 0; i < lineCount; i++) {
		auto line = lines[i];
		if(auto c = strchr(line, '#')){
			*c = '\0';
		}
		line = stb_trimwhite(line);

		char* equals = strchr(line, '=');
		if(!equals) continue;

		*equals = '\0';
		auto name = stb_trimwhite(line);
		auto value = stb_trimwhite(equals+1);
		u32 valueLength = strlen(value);
		if(strlen(name) == 0 || valueLength == 0) continue;

		if(!strcmp("true", value)) {
			options.push_back(Option::Bool(name, true));
		}else if(!strcmp("false", value)) {
			options.push_back(Option::Bool(name, false));
		}else{
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
				options.push_back(Option::Float(name, std::atof(value)));
			}else if(isNumber) {
				options.push_back(Option::Int(name, std::atoll(value)));
			}else{
				options.push_back(Option::String(name, value));
			}
		}

		auto o = &options.back();
		printf("Opt: %s = %s\t\ttype: %hhu\n", o->name, o->value, o->type);
	}

	free(lines);
}

Option* GetOption(const char* oname) {
	auto comp = [oname] (const Option& o) {
		return !strcmp(oname, o.name);
	};

	Option* opt = nullptr;
	auto it = std::find_if(options.begin(), options.end(), comp);
	if(it == options.end()) {
		auto it2 = std::find_if(std::begin(defaultOptions), std::end(defaultOptions), comp);
		if(it2 != std::end(defaultOptions)){
			opt = it2;
		}
	}else{
		opt = &*it;
	}

	return opt;
}

bool GetBoolOption(const char* oname) {
	auto opt = GetOption(oname);

	if(opt) {
		switch(opt->type) {
			case Option::TypeBool: return opt->boolValue;
			case Option::TypeInt: return opt->intValue != 0;
			case Option::TypeFloat: return opt->floatValue != 0.0;
			case Option::TypeString: {
				printf("Warning! Expected option \"%s\" to be a bool, got a string\n", oname);
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
				printf("Warning! Expected option \"%s\" to be an int, got a string\n", oname);
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
				printf("Warning! Expected option \"%s\" to be a float, got a string\n", oname);
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
			puts("Warning! Ran out of space while writing default cfg! Maybe allocate some more space?");
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

	if(!stb_filewritestr(cfgName, buffer)) {
		printf("Warning! Unable to open %s to write defaults\n", cfgName);
	}
}
