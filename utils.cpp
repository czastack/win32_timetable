#include "utils.h"
#include <time.h>
#include <stdio.h> 
#include <cmath>

void trim(STR text)
{
	STR p = text + wcslen(text) - 1;
	while(p >= text && iswspace(*p))
		--p;
	*(p + 1) = 0;
	p = text;
	STR q = p;
	while (*p)
	{
		if (!iswspace(*p))
			*q++ = *p;
		++p;
	}
	*q = 0;
}

void timeStr(STR text)
{
	time_t termStart;
	time_t now = time(0);
	tm t = {0};
	t.tm_year = 2015 - 1900;
	t.tm_mon = 9 - 1;
	t.tm_mday = 7;
	termStart = mktime(&t);

	t = *localtime(&now);
	int week = floor((now - termStart) / (60 * 60 * 24 * 7)) + 1;
	swprintf(text, L"第%d周 %d年%02d月%02d日 星期%c", 
		week, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, L"一二三四五六天"[(t.tm_wday + 6) % 7]);
}

