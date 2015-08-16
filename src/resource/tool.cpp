/**
 * Copyright (c) 2014-2015 Gábor Görzsöny (www.gorzsony.com)
 *
 * This source is a private work and can be used only with the
 * written permission of the author. Do not redistribute it!
 * All rights reserved.
 */

#include <Windows.h>
#include <ShlObj.h>
#include "resource_impl.hpp"

const wchar_t* browseFolder()
{
	struct Callback
	{
		static int CALLBACK proc(HWND hwnd, UINT uMsg, LPARAM, LPARAM lpData)
		{
			if (uMsg == BFFM_INITIALIZED)
			{
				SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			}

			return 0;
		}
	};

	static wchar_t path[MAX_PATH];
	_wgetcwd(path, MAX_PATH);

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Select folder to be packed (gglib format)";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = Callback::proc;
	bi.lParam = (LPARAM)path;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0)
	{
		SHGetPathFromIDList(pidl, path);

		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return nullptr;
}

int main()
{
	gg::ResourceManager& res = static_cast<gg::ResourceManager&>(gg::res);

	std::wstring dir = browseFolder();
	gg::ResourceCreatorPtr creator(res.createResource(dir + L".res"));
	if (creator)
		static_cast<gg::ResourceCreator*>(creator.get())->addDirectory(dir);
	else
		return 1;

	return 0;
}
