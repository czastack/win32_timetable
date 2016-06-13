#pragma once
#include "stdafx.h"
#include <string.h>

struct Course
{
	static const Char None = L'/', Null = L'#';
	Char title[16];
	Char teacher[6];
	Char classRoom[16];
	int startWeek;
	int endWeek;
	void toString(STR buffer);
	void setTitle(CSTR text)
	{
		wcscpy(title, text);
	}
	void setTeacher(CSTR text)
	{
		wcscpy(teacher, text);
	}
	void setClassRoome(CSTR text)
	{
		wcscpy(classRoom, text);
	}
private:
	static CSTR getText(STR text);
};