/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (�) 2012-17 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// This implements the services that form the group chat API

#include "../stdafx.h"

void ShowRoom(SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	if (si->hWnd != nullptr) {
		ActivateExistingTab(si->dat->m_pContainer, si->hWnd);
		return;
	}

	wchar_t szName[CONTAINER_NAMELEN + 2]; szName[0] = 0;
	TContainerData *pContainer = nullptr;
	if (si->dat != nullptr)
		pContainer = si->dat->m_pContainer;
	if (pContainer == nullptr) {
		GetContainerNameForContact(si->hContact, szName, CONTAINER_NAMELEN);
		if (!g_Settings.bOpenInDefault && !mir_wstrcmp(szName, L"default"))
			wcsncpy(szName, L"Chat Rooms", CONTAINER_NAMELEN);
		szName[CONTAINER_NAMELEN] = 0;
		pContainer = FindContainerByName(szName);
	}
	if (pContainer == nullptr)
		pContainer = CreateContainer(szName, FALSE, si->hContact);
	if (pContainer == nullptr)
		return; // smth went wrong, nothing to do here

	MCONTACT hContact = si->hContact;
	if (M.FindWindow(hContact) != 0)
		return;

	if (hContact != 0 && M.GetByte("limittabs", 0) && !wcsncmp(pContainer->szName, L"default", 6)) {
		if ((pContainer = FindMatchingContainer(L"default")) == NULL) {
			wchar_t szName[CONTAINER_NAMELEN + 1];
			mir_snwprintf(szName, L"default");
			if ((pContainer = CreateContainer(szName, CNT_CREATEFLAG_CLONED, hContact)) == NULL)
				return;
		}
	}

	wchar_t *contactName = pcli->pfnGetContactDisplayName(hContact, 0);

	// cut nickname if larger than x chars...
	wchar_t newcontactname[128];
	if (mir_wstrlen(contactName) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, _countof(newcontactname));
		else
			wcsncpy_s(newcontactname, contactName, _TRUNCATE);
	}
	else wcsncpy_s(newcontactname, L"_U_", _TRUNCATE);

	HWND hwndTab = GetDlgItem(pContainer->hwnd, IDC_MSGTABS);

	// hide the active tab
	if (pContainer->hwndActive)
		ShowWindow(pContainer->hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);

	pContainer->iTabIndex = iCount;
	if (iCount > 0) {
		TCITEM item = {};
		for (int i = iCount - 1; i >= 0; i--) {
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, i, &item);
			HWND hwnd = (HWND)item.lParam;
			CSrmmWindow *dat = (CSrmmWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->m_hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->iTabIndex = i;
			}
		}
	}

	TCITEM item = {};
	item.pszText = newcontactname;
	item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	int iTabId = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &item);

	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	TabCtrl_SetCurSel(hwndTab, iTabId);
	pContainer->iChilds++;

	CChatRoomDlg *pDlg = new CChatRoomDlg(si);
	pDlg->m_iTabID = iTabId;
	pDlg->m_pContainer = pContainer;
	pDlg->SetParent(hwndTab);
	pDlg->Create();

	HWND hwndNew = pDlg->GetHwnd();
	
	if (pContainer->dwFlags & CNT_SIDEBAR)
		pContainer->SideBar->addSession(pDlg, pContainer->iTabIndex);

	SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);
	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->hwnd)) {
		SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
		SetFocus(pContainer->hwndActive);
	}

	if (PluginConfig.m_bHideOnClose && !IsWindowVisible(pContainer->hwnd)) {
		WINDOWPLACEMENT wp = { 0 };
		wp.length = sizeof(wp);
		GetWindowPlacement(pContainer->hwnd, &wp);

		BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);			// make sure all tabs will re-check layout on activation
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
		else {
			ShowWindow(pContainer->hwnd, SW_SHOWNORMAL);
		}
		SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
		SetFocus(hwndNew);
	}
	else {
		SetFocus(hwndNew);
		RedrawWindow(pContainer->hwnd, NULL, NULL, RDW_INVALIDATE);
		UpdateWindow(pContainer->hwnd);
		if (GetForegroundWindow() != pContainer->hwnd)
			SetForegroundWindow(pContainer->hwnd);
	}

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M.GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);
}
