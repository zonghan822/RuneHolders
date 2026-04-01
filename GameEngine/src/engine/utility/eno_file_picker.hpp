#pragma once

#include <windows.h>
#include <shobjidl.h>
#include <filesystem>

namespace File_pick
{
	static std::filesystem::path OpenFolderDialog()
	{
		std::filesystem::path result;

		HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED |
			COINIT_DISABLE_OLE1DDE);

		IFileDialog* pFileDialog = nullptr;

		hr = CoCreateInstance(CLSID_FileOpenDialog,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFileDialog));

		if (SUCCEEDED(hr))
		{
			DWORD options;
			pFileDialog->GetOptions(&options);
			pFileDialog->SetOptions(options | FOS_PICKFOLDERS);

			hr = pFileDialog->Show(nullptr);

			if (SUCCEEDED(hr))
			{
				IShellItem* item;
				pFileDialog->GetResult(&item);

				PWSTR path;
				item->GetDisplayName(SIGDN_FILESYSPATH, &path);

				result = path;

				CoTaskMemFree(path);
				item->Release();
			}

			pFileDialog->Release();
		}

		CoUninitialize();
		return result;
	}
}