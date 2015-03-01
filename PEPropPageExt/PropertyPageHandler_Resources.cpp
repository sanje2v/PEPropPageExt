#include "stdafx.h"
#include "PropertyPageHandler.h"
#include "ImageToHBITMAP.h"
#include "IconControl.h"
#include "PictureBoxControl.h"
#include "AnimationControl.h"
#include "EditControl.h"
#include "DialogControl.h"


extern CPEPropPageExtModule g_PEPropPageExtModule;

using namespace ManagedFuncs;

PropertyPageHandler_Resources::PropertyPageHandler_Resources(HWND hWnd, PEReadWrite& PEReaderWriter)
	: PropertyPageHandler(hWnd, std::ref(PEReaderWriter))
{
	m_hTreeViewResourceEntries = GetDlgItem(m_hWnd, IDC_TREERESOURCEENTRIES);
	m_hListViewResourceInfo = GetDlgItem(m_hWnd, IDC_LISTRESOURCEINFO);
	m_hGroupBoxResourcePreview = GetDlgItem(m_hWnd, IDC_STATICRESOURCEPREVIEW);

	// Setup controls
	m_LayoutManager.AddChildConstraint(IDC_TREERESOURCEENTRIES);
	m_LayoutManager.AddChildConstraint(IDC_LISTRESOURCEINFO);
	m_LayoutManager.AddChildConstraint(IDC_STATICRESOURCEPREVIEW, CWA_LEFTRIGHT, CWA_TOPBOTTOM);

	// Set full row selection style for list view
	ListView_SetExtendedListViewStyleEx(m_hListViewResourceInfo,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
										LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP);

	// Insert ListView columns
	LV_COLUMN column;
	ZeroMemory(&column, sizeof(column));

	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText) - 1; ++i)
	{
		column.mask = LVCF_TEXT;
		column.pszText = szGenericColumnText[i];
		ListView_InsertColumn(m_hListViewResourceInfo, i, &column);
	}

	// Resize columns
	for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText) - 1; ++i)
		ListView_SetColumnWidth(m_hListViewResourceInfo,
		                        i,
		                        (i == ARRAYSIZE(szGenericColumnText) - 2 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
}

void PropertyPageHandler_Resources::OnInitDialog()
{
	static const LPWSTR szTreeRoots[] = { L"Native Resources", L"Managed Resources" };

	// Fill them with data
	HTREEITEM arrhRootTreeItems[ARRAYSIZE(szTreeRoots)] = { 0 };
	TVINSERTSTRUCT Item;

	for (size_t i = 0; i < ARRAYSIZE(szTreeRoots); ++i)
	{
		ZeroMemory(&Item, sizeof(Item));
		Item.hParent = TVI_ROOT;
		Item.hInsertAfter = TVI_LAST;
		Item.item.mask = TVIF_TEXT | TVIF_PARAM;
		Item.item.pszText = szTreeRoots[i];
		Item.item.cchTextMax = int(wcslen(szTreeRoots[i]));
		Item.item.lParam = LPARAM(-1L);
		arrhRootTreeItems[i] = TreeView_InsertItem(m_hTreeViewResourceEntries, &Item);

		// Check if there is any CLR Resources
		if (!m_PEReaderWriter.hasCLRResources())
			break;
	}
	
	// Read native resources
	PIMAGE_RESOURCE_DIRECTORY pResDir;
	int err = m_PEReaderWriter.getRootResourceDirectory(std::ref(pResDir));
	if (err)
	{
		LogError(L"ERROR: Couldn't read root resource directory. File is not valid.", true);
		return;
	}
	EnumerateNativeResourceDirAndAddToTreeView(arrhRootTreeItems[0], 0, std::cref(pResDir));

	RTTI::GetTooltipInfo(m_ResourceTooltipInfo, UINT_PTR(NULL), RTTI::RTTI_IMAGE_DOS_HEADER);
	
	// Read managed resources, if any
	unique_ptr<ManagedResourceReader> readerManagedResource(new ManagedResourceReader());
	if (m_PEReaderWriter.hasCLRResources())
	{
		bool failedToLoadDLL;
		
		if (!readerManagedResource->create(m_PEReaderWriter.getFilename().c_str(), std::ref(failedToLoadDLL)))
		{
			if (failedToLoadDLL)
			{
				// NOTE: This project uses supplimentary DLL called 'ManagedFunc.dll' to read managed resources
				LogError(L"WARNING: Couldn't load supplimentary DLL used to read managed resources. Possibly the right .NET Framework has not been installed.\n"
						 L"Managed resources will not be read.", true);
				return;
			}
			else
			{
				LogError(L"ERROR: Managed resources couldn't be read. File is not valid.", true);
				return;
			}
		}

		// Enumerate managed resources
		EnumerateManagedResourceDirAndAddToTreeView(arrhRootTreeItems[1], std::cref(readerManagedResource));

		// Move ownership to class level
		m_readerManagedResource = std::move(readerManagedResource);
	}
}

void PropertyPageHandler_Resources::OnShowWindow()
{
	// Show 'm_pPreviewWindow'
	if (m_pPreviewControl.get())
		m_pPreviewControl->setVisible(true);
}

void PropertyPageHandler_Resources::OnHideWindow()
{
	// Hide 'm_pPreviewWindow'
	if (m_pPreviewControl.get())
		m_pPreviewControl->setVisible(false);
}

wstring PropertyPageHandler_Resources::lstResourceInfo_OnGetTooltip(int Index)
{
	return Generic_OnGetTooltip(m_ResourceTooltipInfo, Index);
}

void PropertyPageHandler_Resources::tvwResourceInfo_OnSelection(HWND hControl, NMTVITEMCHANGE *pItemChange)
{
	m_pPreviewControl.reset();
	ListView_DeleteAllItems(m_hListViewResourceInfo);

	if (pItemChange->lParam == -1)
		return;    // NOTE: Don't do anything for root node

	const NodeDataTypeAndPtr& DataTypeAndData = m_listNodeDataTypeAndPtr[size_t(pItemChange->lParam)];

	switch (DataTypeAndData.rsrcType)
	{
		case ResourceType::Native:
		{
			switch (DataTypeAndData.Type)
			{
				case NodeDataType::ResourceDir:
				{
					PIMAGE_RESOURCE_DIRECTORY pResDir = PIMAGE_RESOURCE_DIRECTORY(DataTypeAndData.u.pEntry);

					m_ResourceInfo =
					{
						{ L"Characteristics", DWORD_toString(pResDir->Characteristics, Hexadecimal) },
						{ L"Time/Date Stamp", DWORD_toString(pResDir->TimeDateStamp) },
						{ L"Version", VersionNums_toString(pResDir->MajorVersion, pResDir->MinorVersion) },
						{ L"No. of Named Entries", WORD_toString(pResDir->NumberOfNamedEntries) },
						{ L"No. of ID Entries", WORD_toString(pResDir->NumberOfIdEntries) }
					};
				}
				break;

				case NodeDataType::ResourceDirEntry:
				{
					PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(DataTypeAndData.u.pEntry);

					m_ResourceInfo =
					{
						{ L"Name/Id", DWORD_toString(pResDirEntry->Name) },
						{ L"Offset to Data/Dir", DWORD_toString(pResDirEntry->OffsetToData, Hexadecimal) }
					};
				}
				break;

				case NodeDataType::ResourceData:
				{
					PIMAGE_RESOURCE_DATA_ENTRY pResData = PIMAGE_RESOURCE_DATA_ENTRY(DataTypeAndData.u.pEntry);

					m_ResourceInfo =
					{
						{ L"Data RVA", DWORD_toString(pResData->OffsetToData, Hexadecimal) },
						{ L"Size", DWORD_toString(pResData->Size) },
						{ L"Code page", DWORD_toString(pResData->CodePage) },
						{ L"Reserved", DWORD_toString(pResData->Reserved) }
					};
				}
				break;
			}

			LV_ITEM item;
			ZeroMemory(&item, sizeof(item));

			for (size_t i = 0; i < m_ResourceInfo.size(); ++i)
			{
				item.iItem = int(i);
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				item.pszText = LPWSTR(_wcsdup(m_ResourceInfo[i].Text.c_str()));
				ListView_InsertItem(m_hListViewResourceInfo, &item);
				free(item.pszText);

				item.iSubItem = 1;
				item.pszText = LPWSTR(_wcsdup(m_ResourceInfo[i].Data.c_str()));
				ListView_SetItem(m_hListViewResourceInfo, &item);
				free(item.pszText);
			}

			// Resize columns
			for (size_t i = 0; i < ARRAYSIZE(szGenericColumnText) - 1; ++i)
				ListView_SetColumnWidth(m_hListViewResourceInfo,
				                        i,
				                        (i == ARRAYSIZE(szGenericColumnText) - 2 ? LVSCW_AUTOSIZE_USEHEADER : LVSCW_AUTOSIZE));
		}
		break;

		case ResourceType::Managed:
		{
			// NOTE: Nothing to display for managed resources
		}
		break;
	}

	TreeViewItemShowData_IfApplicable(m_hTreeViewResourceEntries, pItemChange->hItem);
}

void PropertyPageHandler_Resources::EnumerateNativeResourceDirAndAddToTreeView(HTREEITEM hParentItem,
																			   unsigned int idxNode,
																			   const PIMAGE_RESOURCE_DIRECTORY& pResDir)
{
	assert(hParentItem);

	// Enumerate resource directory and add to tree view
	static const WCHAR szDirectory[] = L"Directory";

	// Add resource directory tag
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));
	item.hParent = hParentItem;
	item.hInsertAfter = TVI_LAST;
	item.item.mask = TVIF_TEXT | TVIF_PARAM;
	item.item.pszText = LPWSTR(szDirectory);
	item.item.cchTextMax = int(ARRAYSIZE(szDirectory));
	m_listNodeDataTypeAndPtr.push_back({ ResourceType::Native, NodeDataType::ResourceDir, pResDir });
	item.item.lParam = LPARAM(m_listNodeDataTypeAndPtr.size() - 1);
	HTREEITEM hNewParentItem = TreeView_InsertItem(m_hTreeViewResourceEntries, &item);

	// Add resource directory entries tags
	const DWORD NumberOfEntries = DWORD(pResDir->NumberOfNamedEntries) + DWORD(pResDir->NumberOfIdEntries);

	int err;
	for (DWORD i = 0; i < NumberOfEntries; ++i)
	{
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;
		err = m_PEReaderWriter.getResourceDirectoryEntry(std::cref(pResDir), i, std::ref(pResDirEntry));
		if (err)
		{
			LogError(L"ERROR: One or more of resource directory entry is not readable. Resource data may be corrupt.", true);
			break;
		}

		wstring EntryNameOrId;
		if (pResDirEntry->NameIsString)
		{
			// Entry has a name string
			wstring EntryName;
			err = m_PEReaderWriter.getResourceEntryName(std::cref(pResDirEntry), std::ref(EntryName));
			if (err)
			{
				LogError(L"ERROR: An resource entry name is unreadable. Resource data may be corrupt.", true);
				EntryName = L"<Unreadable>";
			}

			EntryNameOrId = L'\"' + EntryName + L'\"';
		}
		else
		{
			enum
			{
				EntryNode = 0,
				IdNode,
				LangNode
			};

			// Entry is identified by ID
			const WORD Id = m_PEReaderWriter.getResourceEntryId(std::cref(pResDirEntry));
			const wstring IdString = WORD_toString(Id);
			switch (idxNode)
			{
				case EntryNode:
					EntryNameOrId = L"Entry: " + IdString + L", " + ResourceID_toString(Id);
					break;

				case IdNode:
					EntryNameOrId = L"Id: " + IdString;
					break;

				case LangNode:
				{
					EntryNameOrId = L"Lang: " + IdString;
					wstring LocaleName = LCID_toLocaleName(Id);
					if (!LocaleName.empty())
						EntryNameOrId += L", (" + LocaleName + L')';
				}
				break;

				default:
					EntryNameOrId = IdString;
					break;
			}
		}

		ZeroMemory(&item, sizeof(item));
		item.hParent = hNewParentItem;
		item.hInsertAfter = TVI_LAST;
		item.item.mask = TVIF_TEXT | TVIF_PARAM;
		item.item.pszText = LPWSTR(_wcsdup(EntryNameOrId.c_str()));
		item.item.cchTextMax = int(EntryNameOrId.size());
		m_listNodeDataTypeAndPtr.push_back({ ResourceType::Native, NodeDataType::ResourceDirEntry, pResDirEntry });
		item.item.lParam = LPARAM(m_listNodeDataTypeAndPtr.size() - 1);
		HTREEITEM hNewNewParentItem = TreeView_InsertItem(m_hTreeViewResourceEntries, &item);
		free(item.item.pszText);

		if (pResDirEntry->DataIsDirectory)
		{
			// Entry points to next data directory level
			PIMAGE_RESOURCE_DIRECTORY pNewResDir;
			err = m_PEReaderWriter.getNextResourceDirectory(std::cref(pResDirEntry), std::ref(pNewResDir));
			if (err)
			{
				LogError(L"Error: Couldn't read a resource directory table. Continuing.", true);
				continue;
			}

			EnumerateNativeResourceDirAndAddToTreeView(hNewNewParentItem, idxNode + 1, pNewResDir);
		}
		else
		{
			// Entry points to resource data
			static const WCHAR szData[] = L"Data";
				
			PIMAGE_RESOURCE_DATA_ENTRY pResData;
			err = m_PEReaderWriter.getResourceEntryData(std::cref(pResDirEntry), std::ref(pResData));
			if (err)
			{
				LogError(L"A corrupted resource data entry was encountered. Resource data may be corrupt. Continuing.", true);
				continue;
			}

			ZeroMemory(&item, sizeof(item));
			item.hParent = hNewNewParentItem;
			item.hInsertAfter = TVI_LAST;
			item.item.mask = TVIF_TEXT | TVIF_PARAM;
			item.item.pszText = LPWSTR(szData);
			item.item.cchTextMax = int(ARRAYSIZE(szData));
			m_listNodeDataTypeAndPtr.push_back({ ResourceType::Native, NodeDataType::ResourceData, pResData });
			item.item.lParam = LPARAM(m_listNodeDataTypeAndPtr.size() - 1);
			TreeView_InsertItem(m_hTreeViewResourceEntries, &item);
		}
	}
}

void PropertyPageHandler_Resources::EnumerateManagedResourceDirAndAddToTreeView(HTREEITEM hParentItem,
																				const unique_ptr<ManagedFuncs::ManagedResourceReader>& readerManagedResource)
{
	assert(hParentItem);

	static const WCHAR szStreams[] = L"Streams";

	// Add streams directory tag
	TVINSERTSTRUCT item;
	ZeroMemory(&item, sizeof(item));
	item.hParent = hParentItem;
	item.hInsertAfter = TVI_LAST;
	item.item.mask = TVIF_TEXT | TVIF_PARAM;
	item.item.pszText = LPWSTR(szStreams);
	item.item.cchTextMax = int(ARRAYSIZE(szStreams));
	item.item.lParam = LPARAM(-1);
	HTREEITEM hNewParentItem = TreeView_InsertItem(m_hTreeViewResourceEntries, &item);

	const int NoOfStreamNames = readerManagedResource->getNoOfStreamNames();
	for (int i = 0; i < NoOfStreamNames; ++i)
	{
		WCHAR szStreamName[128];

		// Get stream name
		if (!readerManagedResource->getStreamName(i, szStreamName, ARRAYSIZE(szStreamName)))
		{
			LogError(L"ERROR: Couldn't read managed resource stream name at index " + DWORD_toString(i) + L". Continuing.", true);
			continue;
		}

		// Select stream to enumerate resources in it
		if (!readerManagedResource->selectStream(i))
		{
			LogError(L"ERROR: Couldn't read managed resource stream at index " + DWORD_toString(i) + L". Continuing.", true);
			continue;
		}

		// Add to listview
		ZeroMemory(&item, sizeof(item));
		item.hParent = hNewParentItem;
		item.hInsertAfter = TVI_LAST;
		item.item.mask = TVIF_TEXT | TVIF_PARAM;
		item.item.pszText = LPWSTR(szStreamName);
		item.item.cchTextMax = int(wcslen(szStreamName));
		m_listNodeDataTypeAndPtr.push_back({ ResourceType::Managed, NodeDataType::ResourceDir, i });
		item.item.lParam = LPARAM(m_listNodeDataTypeAndPtr.size() - 1);
		HTREEITEM hNewNewParentItem = TreeView_InsertItem(m_hTreeViewResourceEntries, &item);

		// Enumerate resources in it
		WCHAR szKey[128];
		size_t idxResourceKey = 0;

		while (readerManagedResource->getNextSelectedStreamKeyAndValue(szKey, ARRAYSIZE(szKey)))
		{
			ZeroMemory(&item, sizeof(item));
			item.hParent = hNewNewParentItem;
			item.hInsertAfter = TVI_LAST;
			item.item.mask = TVIF_TEXT | TVIF_PARAM;
			item.item.pszText = LPWSTR(szKey);
			item.item.cchTextMax = int(wcslen(szKey));
			m_listNodeDataTypeAndPtr.push_back({ ResourceType::Managed, NodeDataType::ResourceData, idxResourceKey++ });
			item.item.lParam = LPARAM(m_listNodeDataTypeAndPtr.size() - 1);
			TreeView_InsertItem(m_hTreeViewResourceEntries, &item);
		}
	}
}

void PropertyPageHandler_Resources::TreeViewItemShowData_IfApplicable(HWND hTreeView, HTREEITEM hItem)
{
	assert(hTreeView && hItem);

	// NOTE: This function can display resource data only if:
	//  1. Selected item (and it subsequent children) have just one child OR it is the leaf node
	//  2. We know how to interpret it

	HTREEITEM hChildItem = TreeView_GetChild(hTreeView, hItem);
	if (hChildItem)
	{
		// Here we transverse to last child node

		// Count the no. of siblings of this node
		if (TreeView_GetNextSibling(hTreeView, hChildItem) != NULL)
			return;	// Exit, do nothing if there are siblings

		// Check next level
		TreeViewItemShowData_IfApplicable(hTreeView, hChildItem);
	}
	else
	{
		// This is the last node, so check whether it is a valid data node
		TVITEM item;
		ZeroMemory(&item, sizeof(item));
		item.hItem = hItem;
		item.mask = TVIF_PARAM;

		if (TreeView_GetItem(hTreeView, &item) == FALSE)
			return;

		// Check here whether the item is 'Data' type
		if (m_listNodeDataTypeAndPtr[size_t(item.lParam)].Type != NodeDataType::ResourceData)
			return;

		// NOTE: To figure out the type of data and its encoding
		//  we must transverse item's generation back to the root node
		//  and note the handles of the nodes
		stack<HTREEITEM> stackNodes;
		HTREEITEM hParentItem = hItem;
		do
		{
			stackNodes.push(hParentItem);
		} while (hParentItem = TreeView_GetParent(hTreeView, hParentItem), hParentItem != NULL);

		if (stackNodes.size() < 3)
			return;	// This is not a valid resource directory structure

		stackNodes.pop(); // Remove the root item - it's not needed

		// We need to determine the client area of group box
		// NOTE: 'GetClientRect()' on group box gives us its whole area not just client area
		HDC hDCGroupBoxResourcePreview = GetWindowDC(m_hGroupBoxResourcePreview);
		if (hDCGroupBoxResourcePreview == NULL)
			return;

		// Measure height of caption text of group box
		TEXTMETRIC TextMetric;
		if (GetTextMetrics(hDCGroupBoxResourcePreview, &TextMetric) == FALSE)
		{
			ReleaseDC(m_hGroupBoxResourcePreview, hDCGroupBoxResourcePreview);
			return;
		}

		// Convert according map mode from logical to physical pixels measurement
		POINT pt = { 0, (TextMetric.tmHeight + TextMetric.tmExternalLeading) };
		if (LPtoDP(hDCGroupBoxResourcePreview, &pt, 1) == FALSE)
		{
			ReleaseDC(m_hGroupBoxResourcePreview, hDCGroupBoxResourcePreview);
			return;
		}

		ReleaseDC(m_hGroupBoxResourcePreview, hDCGroupBoxResourcePreview);

		// Ask preview group box for its rectangle
		RECT rectPreviewControl;
		if (GetClientRect(m_hGroupBoxResourcePreview, &rectPreviewControl) == FALSE)
			return;

		// Given that we have total rectangle for the group box, height of its caption text and
		//  a guessed margin for its client area, we can calculate rectangle for its actual client area.
		int MARGIN_TOP = pt.y;
		const int MARGIN = 10;
		POINT pointPreviewControl = { (rectPreviewControl.left + MARGIN), (rectPreviewControl.top + MARGIN_TOP) };
		SIZE sizePreviewControl = { (rectPreviewControl.right - 2 * MARGIN), (rectPreviewControl.bottom - (MARGIN_TOP + MARGIN)) };
		MapWindowPoints(m_hGroupBoxResourcePreview, m_hWnd, &pointPreviewControl, 1);  // Map points from group box to property page window

		// Depending on whether it is an native or managed resource,
		//  handle it appropriately
		switch (m_listNodeDataTypeAndPtr[size_t(item.lParam)].rsrcType)
		{
			case ResourceType::Native:
			{
				enum class DataType
				{
					Binary,
					String,
					StringTable,
					Image,
					Cursor,
					Icon,
					AVI,
					Dialog
				};

				int i = 0;
				DataType typeData = DataType::Binary;
				WORD idData = 0;

				// Tranverse node and take note of it's type, name/Id, language and data
				do
				{
					enum ResNode
					{
						Type = 1,
						Name = 3,
						Lang = 5,
						Data = 6
					};

					ZeroMemory(&item, sizeof(item));
					item.hItem = stackNodes.top();
					item.state = TVIF_PARAM;

					if (TreeView_GetItem(hTreeView, &item) == FALSE)
						return;

					if (item.lParam == -1)
						return;  // Something is wrong, data is missing

					const NodeDataTypeAndPtr& DataTypeAndData = m_listNodeDataTypeAndPtr[size_t(item.lParam)];
					
					int err;
					switch (i)
					{
						case ResNode::Type:
						{
							if (DataTypeAndData.Type != NodeDataType::ResourceDirEntry)
								return;

							// Save data type information
							const PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(DataTypeAndData.u.pEntry);
							
							if (pResDirEntry->NameIsString)
							{
								// Entry has a name string
								wstring EntryName;
								err = m_PEReaderWriter.getResourceEntryName(std::cref(pResDirEntry), std::ref(EntryName));
								if (err)
									return;

								// Name string is case insensitive
								std::transform(EntryName.cbegin(), EntryName.cend(), EntryName.begin(), ::tolower);

								if (EntryName == L"png")
									typeData = DataType::Image;
								else if (EntryName == L"avi")
									typeData = DataType::AVI;
								else if (EntryName == L"registry")
									typeData = DataType::String;
								else if (EntryName == L"xml" || EntryName == L"xslt")
									typeData = DataType::String;
								else
									typeData = DataType::Binary;
							}
							else
							{
								// Entry has an Id
								const int TypeId = m_PEReaderWriter.getResourceEntryId(std::cref(pResDirEntry));

								enum
								{
									Cursor = 1,
									Bitmap,
									Icon,
									Menu,
									Dialog,
									StringTable,
									Accelerator,
									Binary,
									CursorGroup = 12,
									IconGroup = 14,
									Version = 16,
									DialogInclude,
									AnimatedCursor = 21,
									AnimatedIcon,
									HTML,
									Manifest
								};

								switch (TypeId)
								{
									case Cursor:
										typeData = DataType::Cursor;
										break;

									case Icon:
										typeData = DataType::Icon;
										break;

									case Bitmap:
										typeData = DataType::Image;
										break;

									case Dialog:
										typeData = DataType::Dialog;
										break;

									case StringTable:
										typeData = DataType::StringTable;
										break;

									case DialogInclude:
									case HTML:
									case Manifest:
										typeData = DataType::String;
										break;

									case Binary:
									default:
										typeData = DataType::Binary;
								}
							}
						}
						break;

						case ResNode::Name:
						{
							if (DataTypeAndData.Type != NodeDataType::ResourceDirEntry)
								return;

							// Save Id/Name information
							PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(DataTypeAndData.u.pEntry);
							idData = pResEntry->Id;
						}
						break;

						case ResNode::Lang:
						{
							if (DataTypeAndData.Type != NodeDataType::ResourceDirEntry)
								return;

							// Save language information
							PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry = PIMAGE_RESOURCE_DIRECTORY_ENTRY(DataTypeAndData.u.pEntry);
						}
						break;

						case ResNode::Data:
						{
							if (DataTypeAndData.Type != NodeDataType::ResourceData)
								return;

							// Resource data
							PIMAGE_RESOURCE_DATA_ENTRY pResData = PIMAGE_RESOURCE_DATA_ENTRY(DataTypeAndData.u.pEntry);
							
ReEvaluateDataType:
							switch (typeData)
							{
								case DataType::Cursor:
								case DataType::Icon:
								{
									// NOTE: Reading an cursor/icon is little different than reading bitmap. Because I got lazy, I just
									//  use the Windows API to read it
									LPBYTE pData;
									DWORD cData;

									err = m_PEReaderWriter.getResourceData(std::cref(pResData), std::ref(pData), std::ref(cData));
									if (err)
									{
										LogError(L"ERROR: This resource data couldn't be read. Data is corrupted.", true);
										return;
									}

									// LESSON LEARNED: Cannot use 'LoadImage/Icon()' with file mapped as data file
									HICON hIcon = CreateIconFromResourceEx(pData, cData, (typeData == DataType::Icon ? TRUE : FALSE), 0x00030000, 0, 0, LR_DEFAULTCOLOR);

									// Create a static control to display icon
									// CAUTION: PictureBoxControl must be child of property page and NOT group box 'm_hGroupBoxResourcePreview'
									unique_ptr<IconControl> pIconPreview(IconControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pIconPreview.get())
									{
										LogError(L"ERROR: Couldn't show icon preview.", true);

										return;
									}

									// Set size and position
									pIconPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pIconPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set visible
									pIconPreview->setVisible(true);

									// Assign the image
									pIconPreview->setIconHandle(hIcon);

									// Move ownership
									m_pPreviewControl = std::move(pIconPreview);
								}
								break;

								case DataType::Image:
								{
									// Get pointer to image data and size in resource
									LPBYTE pData;
									DWORD cData;

									err = m_PEReaderWriter.getResourceData(std::cref(pResData), std::ref(pData), std::ref(cData));
									if (err)
									{
										LogError(L"ERROR: This resource data couldn't be read. Data is corrupted.", true);
										return;
									}

									HBITMAP hBitmap = createHBITMAPFromImage(pData, cData);  // NOTE: This bitmap will be owned/destroyed by control
									if (!hBitmap)
									{
										LogError(L"ERROR: Couldn't read image resource. Image may be corrupted. It will be shown in hex view.", true);
										
										typeData = DataType::Binary;       // Treat this as unknown binary format
										goto ReEvaluateDataType;
									}

									// Create a picture box control
									// CAUTION: PictureBoxControl must be child of property page and NOT group box 'm_hGroupBoxResourcePreview'
									unique_ptr<PictureBoxControl> pPictureBoxPreview(PictureBoxControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pPictureBoxPreview.get())
									{
										LogError(L"ERROR: Couldn't show picture preview.", true);

										return;
									}
									
									// Set size and position
									pPictureBoxPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pPictureBoxPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set visible
									pPictureBoxPreview->setVisible(true);

									// Assign the image
									pPictureBoxPreview->setBitmapHandle(hBitmap);

									// Move ownership
									m_pPreviewControl = std::move(pPictureBoxPreview);
								}
								break;

								case DataType::AVI:
								{
									// Use animation control to display AVI format data
									unique_ptr<AnimationControl> pAnimationPreview(AnimationControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pAnimationPreview.get())
									{
										LogError(L"ERROR: Couldn't show AVI preview.", true);

										return;
									}

									// Set size and position
									pAnimationPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pAnimationPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set visible
									pAnimationPreview->setVisible(true);

									// Assign animation
									HINSTANCE hMod;
									m_PEReaderWriter.getPrimaryHeader(std::ref(hMod));
									if (!pAnimationPreview->open(hMod, idData))
									{
										LogError(L"ERROR: Couldn't read AVI resource. Data may be corrupted. It will be shown in hex view.", true);

										typeData = DataType::Binary;
										goto ReEvaluateDataType;
									}

									// Move ownership
									m_pPreviewControl = std::move(pAnimationPreview);
								}
								break;

								case DataType::String:
								{
									// Use edit control to display string text
									unique_ptr<EditControl> pTextPreview(EditControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pTextPreview.get())
									{
										LogError(L"ERROR: Couldn't show string preview.", true);

										return;
									}

									// Set size and position
									pTextPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pTextPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set font
									pTextPreview->setFont(L"Consolas", 180);

									// Set visible
									pTextPreview->setVisible(true);

									// Show text string
									LPBYTE pData;
									DWORD cData;
									err = m_PEReaderWriter.getResourceData(std::cref(pResData), std::ref(pData), std::ref(cData));
									if (err)
									{
										LogError(L"ERROR: Couldn't read resource. Data may be corrupted.", true);
										return;
									}
									pTextPreview->setText(MultiByte_toString((const char *)(pData), false, cData).c_str());

									// Move ownership
									m_pPreviewControl = std::move(pTextPreview);
								}
								break;

								case DataType::Dialog:
								{
									// Use the resource dialog template to create dialog
									LPBYTE pTemplate;
									DWORD sizeTemplate;
									err = m_PEReaderWriter.getResourceData(std::cref(pResData), std::ref(pTemplate), std::ref(sizeTemplate));
									if (err)
									{
										LogError(L"ERROR: Couldn't read dialog resource. Data may be corrupted.", true);
										return;
									}

									// Create a dialog from template
									unique_ptr<DialogControl> pDialogPreview(DialogControl::create(g_PEPropPageExtModule.getInstance(), LPCDLGTEMPLATE(pTemplate), sizeTemplate));
									if (!pDialogPreview.get())
									{
										LogError(L"WARNING: Couldn't load dialog box resource. It's likely that the window class associated with the dialog has not been registered yet. "
												 L"It will be shown in hex view.");

										typeData = DataType::Binary;
										goto ReEvaluateDataType;
									}

									// Set visible
									pDialogPreview->setVisible(true);

									// Move ownership
									m_pPreviewControl = std::move(pDialogPreview);
								}
								break;

								case DataType::StringTable:
								{
									LPBYTE pStringTable;
									DWORD sizeStringTable;
									err = m_PEReaderWriter.getResourceData(std::cref(pResData), std::ref(pStringTable), std::ref(sizeStringTable));
									if (err)
									{
										LogError(L"ERROR: Couldn't read string table resource. Data may be corrupted.", true);
										return;
									}

									wstring Strings;
									int idxString = 0;
									int sizeProcessed = 0;

									while (sizeProcessed != sizeStringTable)
									{
										// Read the count of number of UNICODE characters that follow
										WORD NumChars = *(LPWORD(pStringTable));
										pStringTable += sizeof(WORD);

										// Add index number of string as prefix
										Strings += DWORD_toString(idxString) + L' ';

										if (NumChars == 0)
											Strings += L"<Empty>\n";
										else
										{
											// NOTE: In the following statement '+ 1' is added to 'NumChars'
											//  for null-terminating C-String. Also notice '()' for new which
											//  should zero memory and inadvertently add the null-terminator
											Strings += wstring(LPWSTR(pStringTable), NumChars) + L'\n';
											pStringTable += NumChars * sizeof(WCHAR);
										}

										++idxString;
										sizeProcessed += sizeof(WORD) + NumChars * sizeof(WCHAR);
									}

									unique_ptr<EditControl> pTextPreview(EditControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pTextPreview.get())
									{
										LogError(L"ERROR: Couldn't show string preview.", true);

										return;
									}

									// Set text
									pTextPreview->setText(Strings.c_str());

									// Set size and position
									pTextPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pTextPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set font
									pTextPreview->setFont(L"Consolas", 180);

									// Set visible
									pTextPreview->setVisible(true);

									m_pPreviewControl = std::move(pTextPreview);
								}
								break;

								case DataType::Binary:
								default:
								{
									// Use edit control to display binary data
									unique_ptr<EditControl> pBinaryPreview(EditControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
									if (!pBinaryPreview.get())
									{
										LogError(L"ERROR: Couldn't read resource data. It may be corrupted.", true);
										return;
									}

									// Set size and position
									pBinaryPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
									pBinaryPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

									// Set font and tab stops
									pBinaryPreview->setFont(L"Fixedsys", 180);
									int arrTabs[] = { 5, 10 };
									pBinaryPreview->setTabStops(arrTabs, ARRAYSIZE(arrTabs));

									// Set visible
									pBinaryPreview->setVisible(true);

									// Show binary hex view
									wstring HexData;
									DWORD fileOffset = m_PEReaderWriter.RVAToFileOffset(pResData->OffsetToData);
									if (fileOffset == 0)
									{
										LogError(L"ERROR: Couldn't read binary resource. Data may be corrupted.", true);
										return;
									}
									
									err = m_PEReaderWriter.displayHexData(pResData->OffsetToData,
																		  fileOffset,
																		  pResData->Size,
																		  PEReadWrite::HexViewType::Byte,
																		  std::ref(HexData));
									if (err)
									{
										LogError(L"ERROR: Couldn't read binary resource. Data may be corrupted.", true);
										return;
									}
									pBinaryPreview->setText(HexData.c_str());

									// Move ownership
									m_pPreviewControl = std::move(pBinaryPreview);
								}
								break;
							}
						}
						break;
					}

					stackNodes.pop();

				} while (++i, !stackNodes.empty());
			}
			break;

			case ResourceType::Managed:
			{
				int i = 0;
				bool bStreamSelected = false;

				do
				{
					enum
					{
						StreamRoot = 0
					};

					if (i > StreamRoot)
					{
						ZeroMemory(&item, sizeof(item));
						item.hItem = stackNodes.top();
						item.state = TVIF_PARAM;

						if (TreeView_GetItem(hTreeView, &item) == FALSE)
							return;

						if (item.lParam == -1)
							return;

						const NodeDataTypeAndPtr& DataTypeAndData = m_listNodeDataTypeAndPtr[size_t(item.lParam)];

						switch (DataTypeAndData.Type)
						{
							case NodeDataType::ResourceDir:
							{
								if (!m_readerManagedResource->selectStream(int(DataTypeAndData.u.idxStream)))
									return;

								bStreamSelected = true;
							}
							break;

							case NodeDataType::ResourceData:
							{
								if (!bStreamSelected)
									return;

								const size_t idxResourceKey = DataTypeAndData.u.idxStream;

								for (size_t j = 0; j < idxResourceKey; ++j)
								{
									WCHAR szKey[128] = { 0 };

									if (!m_readerManagedResource->getNextSelectedStreamKeyAndValue(szKey, ARRAYSIZE(szKey)))
										return;
								}

								WCHAR szKey[128] = { 0 };
								ManagedFuncs::DataType typeManagedData = ManagedFuncs::DataType::Binary;
								void *pData = NULL;
								int cData = 0;

								if (!m_readerManagedResource->getNextSelectedStreamKeyAndValue(szKey, ARRAYSIZE(szKey), &typeManagedData, &pData, &cData))
									return;

								switch (typeManagedData)
								{
									case DataType::IconHandle:
									{
										if (cData == 1)
										{
											HICON hIcon = HICON(*PUINT_PTR(pData));

											unique_ptr<IconControl> pIconPreview(IconControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
											if (!pIconPreview.get())
												return;

											// Set size and position
											pIconPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
											pIconPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

											// Set visible
											pIconPreview->setVisible(true);

											// Assign the image
											pIconPreview->setIconHandle(hIcon);

											// Move ownership
											m_pPreviewControl = std::move(pIconPreview);
										}
									}
									break;

									case DataType::BitmapHandle:
									{
										if (cData == 1)
										{
											HBITMAP hBitmap = HBITMAP(*PUINT_PTR(pData));

											unique_ptr<PictureBoxControl> pPictureBoxPreview(PictureBoxControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
											if (!pPictureBoxPreview.get())
												return;

											// Set size and position
											pPictureBoxPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
											pPictureBoxPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

											// Set visible
											pPictureBoxPreview->setVisible(true);

											// Assign the image
											pPictureBoxPreview->setBitmapHandle(hBitmap);

											// Move ownership
											m_pPreviewControl = std::move(pPictureBoxPreview);
										}
									}
									break;

									case DataType::String:
									{
										// Use edit control to display string text
										unique_ptr<EditControl> pTextPreview(EditControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
										if (!pTextPreview.get())
										{
											LogError(L"ERROR: Couldn't show string preview.", true);

											return;
										}

										// Set size and position
										pTextPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
										pTextPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

										// Set font
										pTextPreview->setFont(L"Fixedsys", 180);

										// Set visible
										pTextPreview->setVisible(true);

										// Show text string
										pTextPreview->setText((WCHAR *)pData);

										// Move ownership
										m_pPreviewControl = std::move(pTextPreview);
									}
									break;

									case DataType::Binary:
									{
										// Use edit control to display hex data
										unique_ptr<EditControl> pTextPreview(EditControl::create(g_PEPropPageExtModule.getInstance(), m_hWnd));
										if (!pTextPreview.get())
										{
											LogError(L"ERROR: Couldn't show string preview.", true);

											return;
										}

										// Set size and position
										pTextPreview->setPosition(pointPreviewControl.x, pointPreviewControl.y);
										pTextPreview->setSize(sizePreviewControl.cx, sizePreviewControl.cy);

										// Set font
										pTextPreview->setFont(L"Fixedsys", 180);

										// Set visible
										pTextPreview->setVisible(true);

										// Show text string
										wstring hexData;
										displayHexData(LPBYTE(pData), cData, std::ref(hexData));
										pTextPreview->setText(hexData.c_str());

										// Move ownership
										m_pPreviewControl = std::move(pTextPreview);
									}
									break;
								}

								if (pData)
									delete[] pData;
							}
							break;
						}
					}

					stackNodes.pop();

				} while (++i, !stackNodes.empty());
			}
			break;
		}
	}
}

void PropertyPageHandler_Resources::displayHexData(LPBYTE pData, int cData, wstring& out)
{
	static const unsigned int DATA_PER_ROW = 8;
	DWORD address = 0;

	for (int i = 0; i < cData; i += DATA_PER_ROW)
	{
		out += DWORD_toString(DWORD(address), Hexadecimal) + L'\t';

		for (int j = 0; j < DATA_PER_ROW; ++j)
		{
			if (i + j >= cData)
				break;

			out += BYTE_toString(pData[j], Hexadecimal, true, false) + L' ';
		}

		out += L'\t';

		for (int j = 0; j < DATA_PER_ROW; ++j)
		{
			if (i + j >= cData)
				break;

			out += isprint(pData[j]) ? pData[j] : L'.';
			out += L' ';
		}

		pData += DATA_PER_ROW;
		address += DATA_PER_ROW;
		out += L'\n';
	}
}