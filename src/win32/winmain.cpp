/*
 * Copyright (C) 2001  Erik Latoshek [forester] (laterk@inbox.lv)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef WIN32_GUI

#pragma warning(disable : 4047)

#include "common/setup_before.h"
#include "winmain.h"

#include <cstdio>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <process.h>
#include <richedit.h>
#include <cstring>
#include <commctrl.h>
#include <time.h>

#include "common/addr.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/version.h"
#include "common/gui_printf.h"

#include "bnetd/connection.h"
#include "bnetd/account.h"
#include "bnetd/account_wrap.h"
#include "bnetd/ipban.h"
#include "bnetd/message.h"
#include "bnetd/server.h"
#include "bnetd/cmdline.h"
#include "resource.h"
#include "console_output.h"

#include "common/setup_after.h"

#define WM_SHELLNOTIFY          (WM_USER+1)

extern int app_main(int argc, char **argv); /* bnetd main function in bnetd/main.c */

namespace pvpgn
{

	extern HWND	ghwndConsole;

	namespace bnetd
	{

		extern int server_main(int argc, char *argv[]);

		static void	guiThread(void*);
		static void	guiAddText(const char *, COLORREF);
		static void	guiAddText_user(const char *, COLORREF);
		static void	guiDEAD(char*);
		static void	guiMoveWindow(HWND, RECT*);
		static void	guiClearLogWindow(void);
		static void	guiKillTrayIcon(void);

		long PASCAL guiWndProc(HWND, UINT, WPARAM, LPARAM);
		static void	guiOnCommand(HWND, int, HWND, UINT);
		static void	guiOnMenuSelect(HWND, HMENU, int, HMENU, UINT);
		static int	guiOnShellNotify(int, int);
		static BOOL	guiOnCreate(HWND, LPCREATESTRUCT);
		static void	guiOnClose(HWND);
		static void	guiOnSize(HWND, UINT, int, int);
		static void	guiOnPaint(HWND);
		static void	guiOnCaptureChanged(HWND);
		static BOOL	guiOnSetCursor(HWND, HWND, UINT, UINT);
		static void	guiOnMouseMove(HWND, int, int, UINT);
		static void	guiOnLButtonDown(HWND, BOOL, int, int, UINT);
		static void	guiOnLButtonUp(HWND, int, int, UINT);
		static void	guiOnServerConfig(void);
		static void	guiOnAbout(HWND);
		static void	guiOnUpdates(void);
		static void	guiOnAnnounce(HWND);
		static void	guiOnUserStatusChange(HWND);

		BOOL CALLBACK	AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK	AnnDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK	KickDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

#define MODE_HDIVIDE    1
#define MODE_VDIVIDE    1

		struct gui_struc {
			HWND	hwnd;
			HMENU	hmenuTray;
			HWND	hwndUsers;
			HWND	hwndUserCount;
			HWND	hwndUserEditButton;
			HWND	hwndTree;
			int	y_ratio;
			int	x_ratio;
			HANDLE	event_ready;
			BOOL	main_finished;
			int	mode;
			char	szDefaultStatus[128];
			RECT	rectHDivider,
				rectVDivider,
				rectConsole,
				rectUsers,
				rectConsoleEdge,
				rectUsersEdge;
			WPARAM	wParam;
			LPARAM	lParam;
		};

		static struct gui_struc gui;

		char	selected_item[255];

		int fprintf(FILE *stream, const char *format, ...)
		{
			va_list args;
			va_start(args, format);
			if (stream == stderr || stream == stdout)
				return gui_lvprintf(eventlog_level_error, format, args);
			else
				return vfprintf(stream, format, args);
		}

		static void guiThread(void *param)
		{
			WNDCLASSEX wc;
			MSG msg;

			LoadLibrary("RichEd20.dll");
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.style = CS_HREDRAW | CS_VREDRAW;
			wc.lpfnWndProc = (WNDPROC)guiWndProc;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			wc.hInstance = (HINSTANCE)param;
			wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hbrBackground = 0;
			wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
			wc.lpszClassName = "BnetdWndClass";
			wc.hIconSm = NULL;

			if (!RegisterClassEx(&wc)) guiDEAD("cant register WNDCLASS");

			gui.hwnd = CreateWindowEx(
				0,
				wc.lpszClassName,
				"The Player -vs- Player Gaming Network Server",
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				NULL,
				NULL,
				(HINSTANCE)param,
				NULL);

			if (!gui.hwnd) guiDEAD("cant create window");

			ShowWindow(gui.hwnd, SW_SHOW);
			SetEvent(gui.event_ready);
			while (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		long PASCAL guiWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			gui.wParam = wParam;
			gui.lParam = lParam;

			switch (message)
			{
				HANDLE_MSG(hwnd, WM_CREATE, guiOnCreate);
				HANDLE_MSG(hwnd, WM_COMMAND, guiOnCommand);
				HANDLE_MSG(hwnd, WM_MENUSELECT, guiOnMenuSelect);
				HANDLE_MSG(hwnd, WM_SIZE, guiOnSize);
				HANDLE_MSG(hwnd, WM_CLOSE, guiOnClose);
				HANDLE_MSG(hwnd, WM_PAINT, guiOnPaint);
				HANDLE_MSG(hwnd, WM_SETCURSOR, guiOnSetCursor);
				HANDLE_MSG(hwnd, WM_LBUTTONDOWN, guiOnLButtonDown);
				HANDLE_MSG(hwnd, WM_LBUTTONUP, guiOnLButtonUp);
				HANDLE_MSG(hwnd, WM_MOUSEMOVE, guiOnMouseMove);

			case WM_CAPTURECHANGED:
				guiOnCaptureChanged((HWND)lParam);
				return 0;
			case WM_SHELLNOTIFY:
				return guiOnShellNotify(wParam, lParam);
			}

			return DefWindowProc(hwnd, message, wParam, lParam);
		}

		static BOOL guiOnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
		{
			ghwndConsole = CreateWindowEx(
				0,
				RICHEDIT_CLASS,
				NULL,
				WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | ES_NOHIDESEL,
				0, 0,
				0, 0,
				hwnd,
				0,
				0,
				NULL);

			if (!ghwndConsole) return FALSE;

			gui.hwndUsers = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"LISTBOX",
				NULL,
				WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_NOINTEGRALHEIGHT,
				0, 0,
				0, 0,
				hwnd,
				0,
				0,
				NULL);

			if (!gui.hwndUsers) return FALSE;

			//amadeo: temp. button for useredit until rightcklick is working....
			gui.hwndUserEditButton = CreateWindow(
				"button",
				"Edit User Status",
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				0, 0,
				0, 0,
				hwnd,
				(HMENU)881,
				0,
				NULL);

			if (!gui.hwndUserEditButton) return FALSE;

			gui.hwndUserCount = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"edit",
				" 0 user(s) online:",
				WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
				0, 0,
				0, 0,
				hwnd,
				0,
				0,
				NULL);

			if (!gui.hwndUserCount) return FALSE;

			SendMessage(gui.hwndUserCount, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
			SendMessage(gui.hwndUsers, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
			SendMessage(gui.hwndUserEditButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
			BringWindowToTop(gui.hwndUsers);
			strcpy(gui.szDefaultStatus, "Void");

			gui.y_ratio = (100 << 10) / 100;
			gui.x_ratio = (0 << 10) / 100;

			return TRUE;
		}

		static void guiOnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
		{
			switch (id)
			{
				case IDM_EXIT:
					guiOnClose(hwnd);
					break;
				case IDM_SAVE:
					server_save_wraper();
					break;
				case IDM_RESTART:
					server_restart_wraper(restart_mode_all);
					break;
				case IDM_SHUTDOWN:
					server_quit_wraper();
					break;
				case IDM_CLEAR:
					guiClearLogWindow();
					break;
				case IDM_RESTORE:
					guiOnShellNotify(IDI_TRAY, WM_LBUTTONDBLCLK);
					break;
				case IDM_USERLIST:
					guiOnUpdateUserList();
					break;
				case IDM_SERVERCONFIG:
					guiOnServerConfig();
					break;
				case IDM_ABOUT:
					guiOnAbout(hwnd);
					break;
				case ID_HELP_CHECKFORUPDATES:
					guiOnUpdates();
					break;
				case IDM_ANN:
					guiOnAnnounce(hwnd);
					break;
				case ID_USERACTIONS_KICKUSER:
				case 881:
					guiOnUserStatusChange(hwnd);
					break;
			}
		}

		static void guiOnMenuSelect(HWND hwnd, HMENU hmenu, int item, HMENU hmenuPopup, UINT flags)
		{
			// char str[256];
			//by greini 
			/*if( item == 0 || flags == -1)
				SetWindowText( gui.hwndStatus, gui.szDefaultStatus );
				else
				{
				LoadString( GetWindowInstance(hwnd), item, str, sizeof(str) );
				if( str[0] ) SetWindowText( gui.hwndStatus, str );
				}*/
		}

		static int guiOnShellNotify(int uID, int uMessage)
		{
			if (uID == IDI_TRAY) {
				if (uMessage == WM_LBUTTONDBLCLK) {
					if (!IsWindowVisible(gui.hwnd))
						ShowWindow(gui.hwnd, SW_RESTORE);

					SetForegroundWindow(gui.hwnd);
				}
				else if (uMessage == WM_RBUTTONDOWN) {
					POINT cp;
					GetCursorPos(&cp);
					SetForegroundWindow(gui.hwnd);
					TrackPopupMenu(gui.hmenuTray, TPM_LEFTALIGN | TPM_LEFTBUTTON, cp.x, cp.y, 0, gui.hwnd, NULL);
				}
			}
			return 0;
		}

		static void guiOnPaint(HWND hwnd)
		{
			PAINTSTRUCT ps;
			HDC dc;

			dc = BeginPaint(hwnd, &ps);

			DrawEdge(dc, &gui.rectHDivider, BDR_SUNKEN, BF_MIDDLE);
			DrawEdge(dc, &gui.rectConsoleEdge, BDR_SUNKEN, BF_RECT);
			DrawEdge(dc, &gui.rectUsersEdge, BDR_SUNKEN, BF_RECT);

			EndPaint(hwnd, &ps);

			UpdateWindow(ghwndConsole);
			UpdateWindow(gui.hwndUsers);
			UpdateWindow(gui.hwndUserCount);
			UpdateWindow(gui.hwndUserEditButton);
		}

		static void guiOnClose(HWND hwnd)
		{
			guiKillTrayIcon();
			if (!gui.main_finished) {
				eventlog(eventlog_level_debug, __FUNCTION__, "GUI wants server dead...");
				exit(0);
			}
			else {
				eventlog(eventlog_level_debug, __FUNCTION__, "GUI wants to exit...");
				SetEvent(gui.event_ready);
			}
		}

		static void guiOnSize(HWND hwnd, UINT state, int cx, int cy)
		{
			int cy_console, cy_edge, cx_edge, cy_frame, cy_status;

			if (state == SIZE_MINIMIZED) {
				NOTIFYICONDATA dta;

				dta.cbSize = sizeof(NOTIFYICONDATA);
				dta.hWnd = hwnd;
				dta.uID = IDI_TRAY;
				dta.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
				dta.uCallbackMessage = WM_SHELLNOTIFY;
				dta.hIcon = LoadIcon(GetWindowInstance(hwnd), MAKEINTRESOURCE(IDI_ICON1));
				strcpy(dta.szTip, "PvPGN");
				strcat(dta.szTip, " ");
				strcat(dta.szTip, PVPGN_VERSION);
				Shell_NotifyIcon(NIM_ADD, &dta);
				ShowWindow(hwnd, SW_HIDE);
				return;
			}

			if (state == SIZE_RESTORED) {
				NOTIFYICONDATA dta;

				dta.hWnd = hwnd;
				dta.uID = IDI_TRAY;
				Shell_NotifyIcon(NIM_DELETE, &dta);
			}

			cy_status = 0;
			cy_edge = GetSystemMetrics(SM_CYEDGE);
			cx_edge = GetSystemMetrics(SM_CXEDGE);
			cy_frame = (cy_edge << 1) + GetSystemMetrics(SM_CYBORDER) + 1;
			cy_console = ((cy - cy_status - cy_frame - cy_edge * 2)*gui.y_ratio) >> 10;
			gui.rectConsoleEdge.left = 0;
			gui.rectConsoleEdge.right = cx - 140;
			gui.rectConsoleEdge.top = 0;
			gui.rectConsoleEdge.bottom = cy - cy_status;
			gui.rectConsole.left = cx_edge;
			gui.rectConsole.right = cx - 140 - cx_edge;
			gui.rectConsole.top = cy_edge;
			gui.rectConsole.bottom = cy - cy_status;
			gui.rectUsersEdge.left = cx - 140;
			gui.rectUsersEdge.top = 18;
			gui.rectUsersEdge.right = cx;
			gui.rectUsersEdge.bottom = cy - cy_status - 10;
			gui.rectUsers.left = cx - 138;
			gui.rectUsers.right = cx;
			gui.rectUsers.top = 18 + cy_edge;
			gui.rectUsers.bottom = cy - cy_status - 20;
			guiMoveWindow(ghwndConsole, &gui.rectConsole);
			guiMoveWindow(gui.hwndUsers, &gui.rectUsers);
			MoveWindow(gui.hwndUserCount, cx - 140, 0, 140, 18, TRUE);
			MoveWindow(gui.hwndUserEditButton, cx - 140, cy - cy_status - 20, 140, 20, TRUE);
		}

		static BOOL guiOnSetCursor(HWND hwnd, HWND hwndCursor, UINT codeHitTest, UINT msg)
		{
			POINT p;

			if (hwnd == hwndCursor && codeHitTest == HTCLIENT) {
				GetCursorPos(&p);
				ScreenToClient(hwnd, &p);
				if (PtInRect(&gui.rectHDivider, p))
					SetCursor(LoadCursor(0, IDC_SIZENS));

				return TRUE;
			}

			return FORWARD_WM_SETCURSOR(hwnd, hwndCursor, codeHitTest, msg, DefWindowProc);
		}

		static void guiOnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
		{
			POINT p;

			p.x = x;
			p.y = y;

			if (PtInRect(&gui.rectHDivider, p)) {
				SetCapture(hwnd);
				gui.mode |= MODE_HDIVIDE;
			}
		}

		static void guiOnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
		{
			ReleaseCapture();
			gui.mode &= ~(MODE_HDIVIDE | MODE_VDIVIDE);
		}

		static void guiOnCaptureChanged(HWND hwndNewCapture)
		{
			gui.mode &= ~(MODE_HDIVIDE | MODE_VDIVIDE);
		}

		static void guiOnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
		{
			int offset, cy_console, cy_users;
			RECT r;

			if (gui.mode & MODE_HDIVIDE) {
				offset = y - gui.rectHDivider.top;
				if (!offset) return;
				cy_console = gui.rectConsole.bottom - gui.rectConsole.top;
				cy_users = gui.rectUsers.bottom - gui.rectUsers.top;

				if (cy_console + offset <= 0)
					offset = -cy_console;
				else if (cy_users - offset <= 0)
					offset = cy_users;

				cy_console += offset;
				cy_users -= offset;
				if (cy_console + cy_users == 0) return;
				gui.y_ratio = (cy_console << 10) / (cy_console + cy_users);
				GetClientRect(hwnd, &r);
				guiOnSize(hwnd, 0, r.right, r.bottom);
				InvalidateRect(hwnd, NULL, FALSE);
			}
		}

		static void guiOnUpdates()
		{
			ShellExecute(NULL, "open", "http://pvpgn.pro/", NULL, NULL, SW_SHOW);
		}

		static void guiOnAnnounce(HWND hwnd)
		{
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ANN), hwnd, (DLGPROC)AnnDlgProc);
		}

		static void guiOnUserStatusChange(HWND hwnd)
		{
			int  index;
			index = SendMessage(gui.hwndUsers, LB_GETCURSEL, 0, 0);
			SendMessage(gui.hwndUsers, LB_GETTEXT, index, (LPARAM)selected_item);
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_KICKUSER), hwnd, (DLGPROC)KickDlgProc);
			SendMessage(gui.hwndUsers, LB_SETCURSEL, -1, 0);
		}

		static void guiOnAbout(HWND hwnd)
		{
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutDlgProc);
		}

		static void guiOnServerConfig()
		{
			ShellExecute(NULL, NULL, "conf\\bnetd.conf", NULL, NULL, SW_SHOW);
		}

		extern void guiOnUpdateUserList()
		{
			t_connection * c;
			t_elem const * curr;
			t_account * acc;
			char UserCount[80];

			SendMessage(gui.hwndUsers, LB_RESETCONTENT, 0, 0);

			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				if (!(c = (t_connection *)elem_get_data(curr))) continue;
				if (!(acc = conn_get_account(c))) continue;

				SendMessage(gui.hwndUsers, LB_ADDSTRING, 0, (LPARAM)account_get_name(acc));
			}

			sprintf(UserCount, "%d", connlist_login_get_length());
			strcat(UserCount, " user(s) online:");
			SendMessage(gui.hwndUserCount, WM_SETTEXT, 0, (LPARAM)UserCount);
		}

		static void guiAddText(const char *str, COLORREF clr)
		{
			int start_lines, text_length, end_lines;
			CHARRANGE cr;
			CHARRANGE ds;
			CHARFORMAT fmt;
			text_length = SendMessage(ghwndConsole, WM_GETTEXTLENGTH, 0, 0);

			if (text_length > 30000) {
				ds.cpMin = 0;
				ds.cpMax = text_length - 30000;
				SendMessage(ghwndConsole, EM_EXSETSEL, 0, (LPARAM)&ds);
				SendMessage(ghwndConsole, EM_REPLACESEL, FALSE, 0);
			}

			cr.cpMin = text_length;
			cr.cpMax = text_length;

			SendMessage(ghwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr);

			fmt.cbSize = sizeof(CHARFORMAT);
			fmt.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
			fmt.yHeight = 160;
			fmt.dwEffects = 0;
			fmt.crTextColor = clr;
			strcpy(fmt.szFaceName, "Courier New");

			SendMessage(ghwndConsole, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
			SendMessage(ghwndConsole, EM_REPLACESEL, FALSE, (LPARAM)str);
		}

		static void guiDEAD(char *message)
		{
			char *nl;
			char errorStr[4096];
			char *msgLastError;

			FormatMessage(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR)&msgLastError,
				0,
				NULL);

			nl = strchr(msgLastError, '\r');
			if (nl) *nl = 0;

			sprintf(errorStr, "%s\nGetLastError() = '%s'\n", message, msgLastError);

			LocalFree(msgLastError);
			MessageBox(0, errorStr, "guiDEAD", MB_ICONSTOP | MB_OK);
			exit(1);
		}

		static void guiMoveWindow(HWND hwnd, RECT* r)
		{
			MoveWindow(hwnd, r->left, r->top, r->right - r->left, r->bottom - r->top, TRUE);
		}

		static void guiClearLogWindow(void)
		{
			SendMessage(ghwndConsole, WM_SETTEXT, 0, 0);
		}

		static void guiKillTrayIcon(void)
		{
			NOTIFYICONDATA dta;

			dta.cbSize = sizeof(NOTIFYICONDATA);
			dta.hWnd = gui.hwnd;
			dta.uID = IDI_TRAY;
			dta.uFlags = 0;
			Shell_NotifyIcon(NIM_DELETE, &dta);
		}

		BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
		{
			switch (Message) {
			case WM_INITDIALOG:
				return TRUE;
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hwnd, IDOK);
					break;
				}
				break;
			default:
				return FALSE;
			}

			return TRUE;
		}

		BOOL CALLBACK AnnDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
		{
			t_message *message;

			switch (Message) {
			case WM_INITDIALOG:
				return TRUE;
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
				case IDOK:
				{
							 int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_EDIT1));

							 if (len > 0) {
								 char* buf;
								 buf = (char*)GlobalAlloc(GPTR, len + 1);
								 GetDlgItemText(hwnd, IDC_EDIT1, buf, len + 1);

								 if ((message = message_create(message_type_error, NULL, buf))) {
									 message_send_all(message);
									 message_destroy(message);
								 }

								 GlobalFree((HANDLE)buf);
								 SetDlgItemText(hwnd, IDC_EDIT1, "");
							 }
							 else {
								 MessageBox(hwnd, "You didn't enter anything!", "Warning", MB_OK);
							 }
							 break;
				}
				}
				break;
			case WM_CLOSE:
				EndDialog(hwnd, IDOK);
				break;
			default:
				return FALSE;
			}
			return TRUE;
		}

		BOOL CALLBACK KickDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
		{
			switch (Message) {
			case WM_INITDIALOG:
				if (selected_item[0] != 0) {
					SetDlgItemText(hwnd, IDC_EDITKICK, selected_item);
				}

				return TRUE;
			case WM_COMMAND:
				switch (LOWORD(wParam)) {
				case IDC_KICK_EXECUTE:
				{
										 t_connection * conngui;
										 t_account * accountgui;
										 BOOL messageq;
										 BOOL kickq;
										 char temp[60];
										 char ipadr[110];

										 messageq = FALSE;
										 kickq = FALSE;

										 GetDlgItemText(hwnd, IDC_EDITKICK, selected_item, 32);

										 conngui = connlist_find_connection_by_accountname(selected_item);
										 accountgui = accountlist_find_account(selected_item);

										 if (conngui == NULL) {
											 strcat(selected_item, " could not be found in Userlist!");
											 MessageBox(hwnd, selected_item, "ERROR", MB_OK);
										 }
										 else {

											 HWND hButton = GetDlgItem(hwnd, IDC_CHECKBAN);
											 HWND hButton1 = GetDlgItem(hwnd, IDC_CHECKKICK);
											 HWND hButton2 = GetDlgItem(hwnd, IDC_CHECKADMIN);
											 HWND hButton3 = GetDlgItem(hwnd, IDC_CHECKMOD);
											 HWND hButton4 = GetDlgItem(hwnd, IDC_CHECKANN);

											 if (SendMessage(hButton2, BM_GETCHECK, 0, 0) == BST_CHECKED) {
												 account_set_admin(accountgui);
												 account_set_command_groups(accountgui, 255);
												 messageq = TRUE;
											 }

											 if (SendMessage(hButton3, BM_GETCHECK, 0, 0) == BST_CHECKED) {
												 account_set_auth_operator(accountgui, NULL, 1);
												 messageq = TRUE;
											 }

											 if (SendMessage(hButton4, BM_GETCHECK, 0, 0) == BST_CHECKED) {
												 account_set_strattr(accountgui, "BNET\\auth\\announce", "true");
												 messageq = TRUE;
											 }

											 if (SendMessage(hButton, BM_GETCHECK, 0, 0) == BST_CHECKED) {
												 unsigned int	i_GUI;

												 strcpy(temp, addr_num_to_addr_str(conn_get_addr(conngui), 0));

												 for (i_GUI = 0; temp[i_GUI] != ':'; i_GUI++)
													 ipadr[i_GUI] = temp[i_GUI];

												 ipadr[i_GUI] = 0;

												 strcpy(temp, " a ");
												 strcat(temp, ipadr);
												 handle_ipban_command(NULL, temp);

												 temp[0] = 0;
												 strcpy(temp, " has been added to IpBanList");
												 strcat(ipadr, temp);
												 if (messageq == TRUE) {
													 strcat(ipadr, " and UserStatus changed");
													 MessageBox(hwnd, ipadr, "ipBan & StatusChange", MB_OK);
													 messageq = FALSE;
													 kickq = FALSE;
												 }
												 else
													 MessageBox(hwnd, ipadr, "ipBan", MB_OK);
											 }

											 if (SendMessage(hButton1, BM_GETCHECK, 0, 0) == BST_CHECKED) {
												 conn_set_state(conngui, conn_state_destroy);
												 kickq = TRUE;
											 }

											 if ((messageq == TRUE) && (kickq == TRUE)) {
												 strcat(selected_item, "has been kicked and Status has changed");
												 MessageBox(hwnd, selected_item, "UserKick & StatusChange", MB_OK);
											 }

											 if ((kickq == TRUE) && (messageq == FALSE)) {
												 strcat(selected_item, " has been kicked from the server");
												 MessageBox(hwnd, selected_item, "UserKick", MB_OK);
											 }

											 if ((kickq == FALSE) && (messageq == TRUE)) {
												 strcat(selected_item, "'s Status has been changed");
												 MessageBox(hwnd, selected_item, "StatusChange", MB_OK);
											 }

											 selected_item[0] = 0;
										 }
										 break;
				}
				}
				break;
			case WM_CLOSE:
				EndDialog(hwnd, IDC_EDITKICK);
				break;
			default:
				return FALSE;
			}
			return TRUE;
		}

	}

}

using namespace pvpgn;
using namespace pvpgn::bnetd;

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE reserved, LPSTR lpCmdLine, int nCmdShow)
{
	int result;
	Console     console;

	if (cmdline_load(__argc, __argv) != 1) {
		return -1;
	}

	if (cmdline_get_console()){
		console.RedirectIOToConsole();
		return app_main(__argc, __argv);
	}

	pvpgn::bnetd::gui.main_finished = FALSE;
	pvpgn::bnetd::gui.event_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
	_beginthread(pvpgn::bnetd::guiThread, 0, (void*)hInstance);
	WaitForSingleObject(pvpgn::bnetd::gui.event_ready, INFINITE);

	result = app_main(__argc, __argv);

	pvpgn::bnetd::gui.main_finished = TRUE;
	eventlog(pvpgn::eventlog_level_debug, __FUNCTION__, "server exited ( return : %i )", result);
	WaitForSingleObject(pvpgn::bnetd::gui.event_ready, INFINITE);

	return 0;
}

#endif

