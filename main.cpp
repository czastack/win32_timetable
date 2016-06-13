#ifdef _UNICODE
#pragma comment(linker,"/manifestdependency:\"type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#include "stdafx.h"
#include "resource.h"
#include "Course.h"
#include "utils.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

#define PROC_PARAMS HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam
#define PROC_ARGS hwnd, message, wParam, lParam
#define DEF_PROC(method) LRESULT CALLBACK method(PROC_PARAMS)

INT_PTR CALLBACK mainDlgProc(PROC_PARAMS);
INT_PTR CALLBACK courseDlgProc(PROC_PARAMS);
void initMainDlg(HINSTANCE hInstance);
void initTable(HWND hDlg);
void onCourseClick(LPNMITEMACTIVATE pInfo);
bool onCourseUpdate();
void deleteCourse(int courseIndex, int dayIndex);
void clearCourse();
void updateCell(Course* course, int courseIndex, int dayIndex);
void showCourseDlg();
void hideCourseDlg();
void setCourseDlgText(int id, CSTR text);
void readFile();
void saveFile();
void openDir();

CSTR FILE_PATH = L"data.txt";
HWND hCourseDlg;  // 课程设置对话框
HWND hCourseList; // 课程列表控件

int curDayIndex;
int curCourseIndex;
#define Course_LENGTH 4
#define DAY_LENGTH 7
Course* courses[Course_LENGTH][DAY_LENGTH];
CSTR const TIME[] = {
	L"上午第一大节\n7:50 - 10:10",
	L"上午第二大节\n10:30 - 12:00",
	L"下午\n14:40 - 17:00",
	L"晚上\n19:40 - 22:00"
};

/**
 * 程序的入口
 * HINSTANCE hInstance 程序执行模块的句柄(内存中的.exe文件)
 * HINSTANCE hPrevInstance 在Win32程序中总为NULL
 * LPSTR lpCmdLine 一个字符串的命令行参数，不包括程序名
 * int nCmdShow 可能传递给 ShowWindow() 的参数
 */
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    MSG messages;
	initMainDlg(hInstance);
	readFile();

    while (GetMessage (&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

/*初始化主对话框*/
void initMainDlg(HINSTANCE hInstance)
{
	HWND hMainDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), GetDesktopWindow(), mainDlgProc);
	//设置对话框的图标  
	SendMessage(hMainDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, IDI_APPLICATION));
	/*显示时间*/
	Char timeBuf[32]; // 时间字符缓冲区
	timeStr(timeBuf);
	SetWindowText(GetDlgItem(hMainDlg, IDC_TIME), timeBuf);
	//初始化列表控件
	initTable(hMainDlg);
	//显示主对话框
	ShowWindow(hMainDlg, SW_SHOW);

	hCourseDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_COURSE), GetDesktopWindow(), courseDlgProc);
	hideCourseDlg();

}

/**
 * 主对话框过程
 */
INT_PTR CALLBACK mainDlgProc(PROC_PARAMS)
{
	switch(message)
	{
	case WM_CTLCOLORSTATIC:
		return (INT_PTR)((HBRUSH)GetStockObject(WHITE_BRUSH));
	case WM_COMMAND:
		if(HIWORD(wParam) == 0) switch(LOWORD(wParam))
		{
		case IDM_SAVE:
			saveFile();
			break;
		case IDM_LOAD:
			readFile();
			break;
		case IDM_OPENDIR:
			openDir();
			break;
		case IDM_CLEAR:
			clearCourse();
			break;
		} 
		break;
	case WM_NOTIFY:
		if(LOWORD(wParam) == IDC_TABLE && ((LPNMHDR)lParam)->code == NM_DBLCLK)
			onCourseClick((LPNMITEMACTIVATE)lParam);
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_CLOSE)
			PostQuitMessage(0); //退出
		break;
	}
	return 0;
}

/**
 * 课程设置对话框过程
 */
INT_PTR CALLBACK courseDlgProc(PROC_PARAMS)
{
	switch(message)
	{
	case WM_INITDIALOG:
		// 设置对话框的图标  
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(NULL, IDI_APPLICATION));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_OK:
			if(onCourseUpdate())
				hideCourseDlg();
			break;
		case ID_CANCEL:
			hideCourseDlg();
			break;
		case ID_DELETE:
			deleteCourse(curCourseIndex, curDayIndex);
			hideCourseDlg();
			break;
		}
		break;
	case WM_SYSCOMMAND:
		if(wParam == SC_CLOSE)
			hideCourseDlg(); //隐藏
		break;
	}
	return 0;
}

/**
 * 初始化列表控件
 */
void initTable(HWND hDlg)
{
	HWND table = GetDlgItem(hDlg, IDC_TABLE);
	hCourseList = table;

	ListView_SetBkColor(table, 0xECF0F1);
	ListView_SetExtendedListViewStyle(table, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	HIMAGELIST hImageList = ImageList_Create(1, 100, ILC_COLOR16 | ILC_MASK, 1, 1);
	ListView_SetImageList(table, hImageList, LVSIL_STATE);

	// 绘制表头
	LV_COLUMN column = {LVCF_TEXT | LVCF_WIDTH | LVCF_FMT, LVCFMT_CENTER, 160};
	column.pszText = L"时间";
	ListView_InsertColumn(table, 1, &column);

	CSTR headers = L"一二三四五六天";
	Char headerBuffer[] = L"星期*";
	for (int i = 0; i < DAY_LENGTH;)
	{
		headerBuffer[2] = headers[i];
		column.pszText = headerBuffer;
		ListView_InsertColumn(table, ++i, &column);
	}

	// 插入行头
	LVITEM item = {LVIF_IMAGE | LVIF_TEXT, 0, 0};
	while (item.iItem < Course_LENGTH)
	{
		ListView_InsertItem(table, &item);
		item.pszText = (STR)TIME[item.iItem];
		ListView_SetItem(table, &item);
		++item.iItem;
	}
}

/**
 * 课程单元格双击事件
 */
void onCourseClick(LPNMITEMACTIVATE pInfo)
{
	if (pInfo->iSubItem == 0)
		return;
	curDayIndex = pInfo->iSubItem - 1;
	curCourseIndex = pInfo->iItem;

	Char buffer[3];
	Course* &course = courses[curCourseIndex][curDayIndex];
	if (course)
	{
		// 同步对话框中的内容
		setCourseDlgText(IDC_NAME, course->title);
		setCourseDlgText(IDC_TEACHER, course->teacher);
		setCourseDlgText(IDC_CLASSROOM, course->classRoom);
		wsprintf(buffer, L"%d", course->startWeek);
		setCourseDlgText(IDC_WEEK_START, buffer);
		wsprintf(buffer, L"%d", course->endWeek);
		setCourseDlgText(IDC_WEEK_END, buffer);
	}
	else
	{
		setCourseDlgText(IDC_NAME, nullptr);
		setCourseDlgText(IDC_TEACHER, nullptr);
		setCourseDlgText(IDC_CLASSROOM, nullptr);
		setCourseDlgText(IDC_WEEK_START, nullptr);
		setCourseDlgText(IDC_WEEK_END, nullptr);
	}
	showCourseDlg();
}

/**
 * 完成修改课程
 */
bool onCourseUpdate()
{
	Char buffer[16];
	CSTR error = nullptr;
	Course* &course = courses[curCourseIndex][curDayIndex];
	auto getText = [&buffer](int id) -> CSTR {
		GetWindowText(GetDlgItem(hCourseDlg, id), buffer, 16);
		trim(buffer);
		return buffer;
	};
	if(!course)
		course = new Course;
	course->setTitle(getText(IDC_TITLE));
	if(!*course->title)
		error = L"请输入标题";
	if(!error)
	{
		course->setTeacher(getText(IDC_TEACHER));
		if(!*course->teacher)
			error = L"请输入教师";
	}
	if(!error)
	{
		course->setClassRoome(getText(IDC_CLASSROOM));
		if(!*course->classRoom)
			error = L"请输入教室";
	}
	if(!error)
	{
		course->startWeek = _wtoi(getText(IDC_WEEK_START));
		if(course->startWeek == 0)
			error = L"起始周不能为空或0";
	}
	if(!error)
	{
		course->endWeek = _wtoi(getText(IDC_WEEK_END));
		if(course->endWeek == 0)
			error = L"结束周不能为空或0";
	}
	if (error)
	{
		SetWindowText(GetDlgItem(hCourseDlg, IDC_ERROR), error);
		return false;
	}
	else
	{
		updateCell(course, curCourseIndex, curDayIndex);
		return true;
	}
}

/*删除课程*/
void deleteCourse(int courseIndex, int dayIndex)
{
	Course* &course = courses[courseIndex][dayIndex];
	if (course)
	{
		delete course;
		course = nullptr;
		updateCell(course, courseIndex, dayIndex);
	}
}

/*清空课程*/
void clearCourse()
{
	for (int courseIndex = 0; courseIndex < Course_LENGTH; ++courseIndex)
	{
		for (int dayIndex = 0; dayIndex < DAY_LENGTH; ++dayIndex)
		{
			deleteCourse(courseIndex, dayIndex);
		}
	}
}

/**
 * 更新单元格显示
 */
void updateCell(Course* course, int courseIndex, int dayIndex)
{
	LVITEM item = {LVIF_TEXT, courseIndex, dayIndex + 1};
	if (course)
	{
		Char buffer[32];
		course->toString(buffer);
		item.pszText = buffer;
	}
	ListView_SetItem(hCourseList, &item);
}

/*显示课程设置对话框*/
void showCourseDlg()
{
	setCourseDlgText(IDC_ERROR, L"请用/表示空");
	ShowWindow(hCourseDlg, SW_SHOW);
	SetActiveWindow(hCourseDlg);
}
/*隐藏课程设置对话框*/
void hideCourseDlg()
{
	ShowWindow(hCourseDlg, SW_HIDE);
}
/*设置课程对话框文本*/
void setCourseDlgText(int id, CSTR text)
{
	SetWindowText(GetDlgItem(hCourseDlg, id), text);
}

/**
 * 从文件读取数据
 */
void readFile()
{
	FILE *fp = _wfopen(FILE_PATH, L"r,ccs=UNICODE");
	if(!fp)
		return;
	for (int courseIndex = 0; courseIndex < Course_LENGTH; ++courseIndex)
	{
		if(feof(fp))
			break;
		Char buffer[16];
		for (int dayIndex = 0; dayIndex < DAY_LENGTH; ++dayIndex)
		{
			Course* &course = courses[courseIndex][dayIndex];
			fwscanf(fp, L"%s", buffer);
			if (*buffer != Course::Null)
			{
				if(!course)
					course = new Course;
				course->setTitle(buffer);
				fwscanf(fp, L"%s%s%d%d", course->classRoom, course->teacher, 
					&course->startWeek, &course->endWeek);
				updateCell(course, courseIndex, dayIndex);
			}
		}
	}
	fclose(fp);
}

/**
 * 数据写入到文件
 */
void saveFile()
{
	FILE *fp = _wfopen(FILE_PATH, L"w,ccs=UNICODE");
	if(!fp)
		return;
	for (int courseIndex = 0; courseIndex < Course_LENGTH; ++courseIndex)
	{
		for (Course* &course: courses[courseIndex])
		{
			if(course)
			{
				fwprintf(fp, L"%s\t%s\t%s\t%d\t%d\n", 
					course->title, course->classRoom, course->teacher,
					course->startWeek, course->endWeek);
			} else
				fwprintf(fp, L"%c\n", Course::Null);
		}
		fwprintf(fp, L"\n");
	}
	fclose(fp);
}

/*打开文件夹*/
void openDir()
{
	Char buf[128];
	GetCurrentDirectory(sizeof(buf), buf);
	ShellExecute(NULL, L"open", L"explorer", buf, NULL, SW_SHOWNORMAL);
}
