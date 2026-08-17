#ifndef INCLUDED_UTILS_H
#define INCLUDED_UTILS_H


namespace pvpgn
{

	namespace d2gs
	{

		extern char** strtoarray(char const* str, char const* delim, int* count);
		extern char** strtoargv(char const* str, int* count);
		extern char* str_strip_affix(char* str, char const* affix);
		extern char* string_color(char* str);

	}

}


#endif
