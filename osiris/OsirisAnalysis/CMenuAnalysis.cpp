/*
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*  FileName: CMenuAnalysis.cpp
*  Author:   Douglas Hoffman
*
*/
#include "mainApp.h"
#include "CMenuAnalysis.h"
#include "CMenuFileAnalysis.h"
#include "CMenuLabels.h"
#include "CMenuEdit.h"
#include "CMenuSort.h"
#include "COARfile.h"
#include "CMenuHistory.h"
#include "CComboLabels.h"
#include "CMenuBar.h"
#include "CMDIFrame.h"
#include "wxIDS.h"


#define SHOW_PREVIEW wxS("Show Preview")
#define HIDE_PREVIEW wxS("Hide Preview")
const wxChar *CMenuAnalysis::PARAMETERS_TOOL_TIP =
	wxS("View parameters used to create this analysis.");

const wxChar *CMenuAnalysis::PREVIEW_HELP_TEXT =
  wxS("Show or hide preview plot of selected table cell");

#define POPUP_ECHO(f,a) if(m_pMenuPopup != NULL) { m_pMenuPopup->f(a); }
#define HIST_ECHO(f,a) if(m_pMenuHistoryButton != NULL) { m_pMenuHistoryButton->f(a); }

CMenuAnalysis::~CMenuAnalysis()
{
#define _DELETE(x) if(x != NULL) { delete x; }
  _DELETE(m_pMenuPopup);
  _DELETE(m_pMenuHistoryButton);
  _DELETE(m_pMenuSortPopup);
  _DELETE(m_pMenuEdit);
#undef _DELETE
}

void CMenuAnalysis::SetControlsOnTop(bool b)
{
  m_pMenuSort->SetControlsOnTop(b);
  POPUP_ECHO(SetControlsOnTop,b);
  if(m_pMenuSortPopup != NULL)
  {
    m_pMenuSortPopup->SetControlsOnTop(b);
  }
}

void CMenuAnalysis::SetPreviewTextShow(bool bShow)
{
  const wxChar *p = bShow ? SHOW_PREVIEW : HIDE_PREVIEW;
  m_pMenuItemPreview->SetItemLabel(p);
  POPUP_ECHO(SetPreviewTextShow,bShow)
}
void CMenuAnalysis::SetCellTypeFromComboBox(CComboLabels *p)
{
  m_pMenuCellType->SelectByComboBox(p);
  POPUP_ECHO(SetCellTypeFromComboBox,p)
}
void CMenuAnalysis::SetNameTypeFromComboBox(CComboLabelsName *p)
{
  m_pMenuCellType->SelectByComboBox(p,true);
  POPUP_ECHO(SetNameTypeFromComboBox,p)
}
void CMenuAnalysis::SetNameTypeFromId(int nID)
{
  m_pMenuCellType->Check(nID,true);
  POPUP_ECHO(SetNameTypeFromId,nID)
}

void CMenuAnalysis::EnableHistoryView(bool b)
{
  m_pMenuHistory->EnableCell(b);
  HIST_ECHO(EnableCell,b);
  POPUP_ECHO(EnableHistoryView,b);
}
void CMenuAnalysis::UpdateHistory(bool bForce)
{
  m_pMenuHistory->UpdateList(bForce);
  HIST_ECHO(UpdateList,bForce);
  POPUP_ECHO(UpdateHistory,bForce);
}
bool CMenuAnalysis::SelectTime(const wxDateTime *pTime)
{
  bool bRtn = m_pMenuHistory->SelectTime(pTime);
  HIST_ECHO(SelectTime,pTime);
  POPUP_ECHO(SelectTime,pTime);
  return bRtn;
}
void CMenuAnalysis::SetHistoryCellLabel(const wxString &s)
{
  m_pMenuHistory->SetCellLabel(s);
  HIST_ECHO(SetCellLabel,s);
  POPUP_ECHO(SetHistoryCellLabel,s);
}


// virtual functions

void CMenuAnalysis::EnableAccept(bool bLocus, bool bSample, bool bILS, bool bChannels, bool bDir)
{
  CBaseMenuEdit::EnableAccept(bLocus, bSample, bILS, bChannels, bDir);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->EnableAccept(bLocus, bSample, bILS, bChannels, bDir);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->EnableAccept(bLocus, bSample, bILS, bChannels, bDir);
  }
}
void CMenuAnalysis::EnableReview(bool bLocus, bool bSample, bool bILS, bool bChannels, bool bDir)
{
  CBaseMenuEdit::EnableReview(bLocus, bSample, bILS, bChannels, bDir);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->EnableReview(bLocus, bSample, bILS, bChannels, bDir);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->EnableReview(bLocus, bSample, bILS, bChannels, bDir);
  }
}
void CMenuAnalysis::SetSampleEnabled(bool bEnable,bool bAllowDisable)
{
  CBaseMenuEdit::SetSampleEnabled(bEnable,bAllowDisable);
  bool bHasDisabled = m_pFile->SamplesDisabled();
  Enable(IDmenuReAnalyze,bHasDisabled);
  Enable(IDmenuDeleteDisabled,bHasDisabled);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->SetSampleEnabled(bEnable,bAllowDisable);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->SetSampleEnabled(bEnable,bAllowDisable);
  }
}
void CMenuAnalysis::SetLocus(const wxString &s, bool bEnabled)
{
  CBaseMenuEdit::SetLocus(s,bEnabled);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->SetLocus(s,bEnabled);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->SetLocus(s,bEnabled);
  }
}
void CMenuAnalysis::EnableEditLocus(bool bEnable)
{
  CBaseMenuEdit::EnableEditLocus(bEnable);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->EnableEditLocus(bEnable);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->EnableEditLocus(bEnable);
  }
}

void CMenuAnalysis::EnableEditDirectory(bool bEnable)
{
  CBaseMenuEdit::EnableEditDirectory(bEnable);
  if(m_pMenuEdit != NULL)
  {
    m_pMenuEdit->EnableEditDirectory(bEnable);
  }
  if(m_pMenuPopup != NULL)
  {
    m_pMenuPopup->EnableEditDirectory(bEnable);
  }
}


bool CMenuAnalysis::SetCellTypeOffset(int n)
{
  bool bRtn = m_pMenuCellType->SelectByOffset(n);
  POPUP_ECHO(SetCellTypeOffset,n);
  return bRtn;
}
bool CMenuAnalysis::SetNameTypeOffset(int n)
{
  bool bRtn = m_pMenuCellType->SelectByOffset(n,true);
  POPUP_ECHO(SetNameTypeOffset,n);
  return bRtn;
}
void CMenuAnalysis::EnablePreview(bool bEnable)
{
  m_pMenuItemPreview->Enable(bEnable);
  POPUP_ECHO(EnablePreview,bEnable);
}



int CMenuAnalysis::GetCellTypeOffset()
{
  int n = m_pMenuCellType->GetCheckedOffset();
  return n;
}
int CMenuAnalysis::GetNameTypeOffset()
{
  int n = m_pMenuCellType->GetCheckedOffset(true);
  return n;
}
int CMenuAnalysis::GetCellTypeOffsetById(int nID)
{
  int nRtn = m_pMenuCellType->GetOffsetById(nID);
  return nRtn;
}
int CMenuAnalysis::GetNameTypeOffsetById(int nID)
{
  int nRtn = m_pMenuCellType->GetOffsetById(nID,true);
  return nRtn;
}

CMenuAnalysis::CMenuAnalysis(COARfile *pFile) :
  m_pMenuPopup(NULL),
  m_pMenuHistoryButton(NULL),
  m_pMenuSortPopup(NULL),
  m_pMenuEdit(NULL)
{
  m_pFile = pFile;
  m_pMenuSort = new CMenuSort;
  m_pMenuHistory = new CMenuHistory(pFile,true);
  Append(
    IDmenuDisplayGraph,
    "Display Graph",
    "Display channel plots.  Hold the shift key down for a single plot"
    );
  Append(
    IDmenuDisplaySample,
    "Display Sample",
    "Display all details for the current sample"
    );
  Append(
    IDmenuSampleTile,
    "Display Sample and Graph\tCtrl+G",
    "Display all details for the current sample"
    );
  m_pMenuItemPreview = Append(
    IDmenuTogglePreview,
    SHOW_PREVIEW,
    PREVIEW_HELP_TEXT);
  Append(
    IDmenuEditDirectory,
    CMenuEdit::LABEL_EDIT_DIRECTORY
    );
  Append(
    IDmenuEditCell,
    ".",
    "Edit data selected below"
    );
  m_pMenuAccept = new CMenuAccept;
  m_pMenuItemAccept = AppendSubMenu(
    m_pMenuAccept,CMenuEdit::LABEL_ACCEPT_ALERTS);
  m_pMenuReview = new CMenuReview;
  m_pMenuItemReview = AppendSubMenu(
    m_pMenuReview,CMenuEdit::LABEL_REVIEW_EDIT);
  m_pMenuItemDisable = Append(IDmenuDisableSample,".");
  m_pMenuItemDisableMulti = Append(IDmenuDisableMultiple,"Disable/Enable Multiple...\tAlt+D");
  m_pMenuItemReAnalyze = Append(
    IDmenuReAnalyze,
    "Reanalyze Disabled...",
    "Reanalyze disabled samples");
  m_pMenuItemDeleteDisabled = Append(
    IDmenuDeleteDisabled,
    "Delete Disabled Samples...",
    "Delete disabled samples");
  AppendSubMenu(
    m_pMenuHistory,"&History");
  AppendSubMenu(m_pMenuSort,"&Sort");
  Append(IDmenuParameters,"Parameters...",
    PARAMETERS_TOOL_TIP);
  m_pMenuCellType = new CMenuLabels(CMenuLabels::MENU_TYPE_TABLE);
  AppendSubMenu(m_pMenuCellType,"&Display");
  Append(
    IDExportCMF,
    CMenuFileAnalysis::EXPORT_CMF,
    CMenuFileAnalysis::EXPORT_CMF_HELP);
  m_pMenuItemToolbar = Append(IDmenuShowHideTableToolbar,CMDIFrame::HIDE_TOOLBAR);
}

CMenuAnalysis *CMenuAnalysis::Clone()
{
  CMenuAnalysis *pRtn = new CMenuAnalysis(m_pFile);
  CopyState(pRtn);
  return pRtn;
}
void CMenuAnalysis::CopyState(CMenuAnalysis *pTo)
{
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemPreview,m_pMenuItemPreview);
  CopyStateEdit(pTo);
  m_pMenuHistory->CopyState(pTo->m_pMenuHistory);
  pTo->SetControlsOnTop(m_pMenuSort->GetControlsOnTop());
  pTo->m_pMenuCellType->SelectByOffset(
    m_pMenuCellType->GetCheckedOffset());
  pTo->m_pMenuCellType->SelectByOffset(
    m_pMenuCellType->GetCheckedOffset(true),true);
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemToolbar,m_pMenuItemToolbar);
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemDisable,m_pMenuItemDisable);
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemDisableMulti,m_pMenuItemDisableMulti);
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemReAnalyze,m_pMenuItemReAnalyze);
  CBaseMenuEdit::CopyState(pTo->m_pMenuItemDeleteDisabled,m_pMenuItemDeleteDisabled);
  pTo->UpdateChildren();
}

wxMenu *CMenuAnalysis::GetMenuSortPopup()
{
  return _GetMenuSortPopup();
}
CMenuHistory *CMenuAnalysis::GetMenuHistoryPopup()
{
  return _GetHistoryPanelPopup();
}

wxMenu *CMenuAnalysis::GetMenuEdit()
{
  if(m_pMenuEdit == NULL)
  {
    m_pMenuEdit = new CMenuEdit;
    CopyStateEdit(m_pMenuEdit);
  }
  return m_pMenuEdit;
}
wxMenu *CMenuAnalysis::GetPopup()
{
  return _GetPopup();
}



void CMenuAnalysis::_UpdatePopup()
{
  if(m_pMenuPopup != NULL)
  {
    CopyState(m_pMenuPopup);
    m_pMenuPopup->UpdateChildren();
  }
}
void CMenuAnalysis::_UpdateHistoryButton()
{
  if(m_pMenuHistoryButton != NULL)
  {
    m_pMenuHistory->CopyState(m_pMenuHistoryButton);
  }
}
wxMenu *CMenuAnalysis::_GetPopup()
{
  if(m_pMenuPopup == NULL)
  {
    m_pMenuPopup = Clone();
  }
  return m_pMenuPopup;
}
CMenuHistory *CMenuAnalysis::_GetHistoryPanelPopup()
{
  if(m_pMenuHistoryButton == NULL)
  {
    m_pMenuHistoryButton = new CMenuHistory(m_pFile,true);
    _UpdateHistoryButton();
  }
  return m_pMenuHistoryButton;
}
wxMenu *CMenuAnalysis::_GetMenuSortPopup()
{
  if(m_pMenuSortPopup == NULL)
  {
    m_pMenuSortPopup = new CMenuSort();
    m_pMenuSortPopup->SetControlsOnTop(m_pMenuSort->GetControlsOnTop());
  }
  return m_pMenuSortPopup;
}

void CMenuAnalysis::UpdateChildren()
{
  _UpdatePopup();
  _UpdateHistoryButton();
}

