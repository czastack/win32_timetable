#include "Course.h"
#include "windows.h"

void Course::toString(STR buffer)
{
	wsprintf(buffer, L"%s\n%d-%d÷‹ / %s", 
		getText(title), startWeek, endWeek, getText(classRoom));
}

CSTR Course::getText(STR text)
{
	return *text == None ? nullptr : text;
}
