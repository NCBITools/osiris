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
*  FileName: CPanelLocusDetails.cpp
*  Author:   Douglas Hoffman
*
*/
#include "mainApp.h"
#include "CPanelLocusDetails.h"
#include "CPanelUserID.h"
#include <wx/stattext.h>
#include <wx/sizer.h>
#include "nwx/nwxString.h"
#include "nwx/nwxColorUtil.h"
#include "wxIDS.h"
#include "Platform.h"
#include "CLabels.h"

IMPLEMENT_ABSTRACT_CLASS(CPanelLocusDetails,wxPanel)

CPanelLocusDetails::~CPanelLocusDetails() {;}
CPanelLocusDetails::CPanelLocusDetails(
  const COARlocus *pLocus,
  const COARmessages *pMessages,
  wxWindow *parent,
  wxWindowID id,
  bool bSplitHorizontal,
  bool bReadOnly,
  bool bReadOnlyMayChange) :
    wxPanel(parent,id),
//    m_LocusEdit(*pLocus), // init replaced by _InitData()
//    m_MsgEdit(*pMessages),
    m_pPanelUser(NULL),
    m_pLocus(pLocus),
    m_pSample(NULL),
    m_pMsgs(pMessages),
    m_bReadOnly(bReadOnly),
    m_bReadOnlyMayChange(bReadOnlyMayChange) 
     // if read only and bReadOnlyMayChange == true,
     // a "new notes" text box is created
{
  _InitCommon(bSplitHorizontal);
}
CPanelLocusDetails::CPanelLocusDetails(
  const COARsample *pSample,
  int nChannel,
  const COARlocus *pLocus,
  const COARmessages *pMessages,
  wxWindow *parent,
  wxWindowID id,
  bool bSplitHorizontal,
  bool bSplitLocusHorizontal,
  bool bReadOnly,
  bool bReadOnlyMayChange) :
    wxPanel(parent,id),
//    m_LocusEdit(*pLocus), // init replaced by _InitData()
//    m_MsgEdit(*pMessages),
    m_pPanelUser(NULL),
    m_pLocus(pLocus),
    m_pSample(pSample),
    m_pMsgs(pMessages),
    m_bReadOnly(bReadOnly),
    m_bReadOnlyMayChange(bReadOnlyMayChange)
{
  _InitCommon(
    bSplitHorizontal,bSplitLocusHorizontal,
    pSample,nChannel);
}


void CPanelLocusDetails::_InitCommon(
  bool bSplitHorizontal, 
  bool bSplitLocusHorizontal,
  const COARsample *pSample,
  int nChannel)
{
  wxPanel *pPanelNotes;
  wxBoxSizer *pSizer;
  wxPanel *pPanelLocus;
  //m_MsgEdit.CopyOnly(*m_pMsgs,m_LocusEdit.GetAlerts()); // replaced with _InitData()
  _InitData();
  int nStyle = ID_SPLITTER_STYLE;
  if(!bSplitHorizontal)
  {
    nStyle |= wxSP_BORDER;
  }
  m_pSplitter = new wxSplitterWindow(this,wxID_ANY,
    wxDefaultPosition,wxDefaultSize, nStyle);
  pPanelLocus = new wxPanel(m_pSplitter,wxID_ANY);
  pPanelNotes = _CreateNotesPanel();
  m_pSplitterLocus = new wxSplitterWindow(pPanelLocus,wxID_ANY,
    wxDefaultPosition, wxDefaultSize, ID_SPLITTER_STYLE);

  if(nChannel < 0)
  {
    m_pGridLocus11 = new CGridLocus(
      &m_LocusEdit,m_pSplitterLocus,wxID_ANY,m_bReadOnly);
    m_pGridLocus = m_pGridLocus11;
    m_pGridLocusPeaks = NULL;
  }
  else
  {
    m_pGridLocusPeaks = new CGridLocusPeaks(pSample,
      nChannel,m_pLocus->GetName(),
      m_pSplitterLocus,wxID_ANY,m_bReadOnly);
    m_pGridLocus = m_pGridLocusPeaks;
    m_pGridLocus11 = NULL;
  }
  m_pGridAlerts = new CGridAlerts(
    &m_MsgEdit,m_pSplitterLocus,wxID_ANY,0,m_bReadOnly);

  pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(m_pSplitterLocus,1, wxEXPAND | wxALL,   
    0);
  pPanelLocus->SetSizer(pSizer);

#if 0
  pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(m_pSplitter,1, wxEXPAND);
  SetSizer(pSizer);
  pSizer->Layout();
#endif

  double dGravity = 0.5;
  if(bSplitLocusHorizontal)
  {
    m_pSplitterLocus->SplitHorizontally(m_pGridLocus,m_pGridAlerts,256);
    dGravity = 0.0;
  }
  else
  {
    m_pSplitterLocus->SplitVertically(m_pGridLocus,m_pGridAlerts,0);
  }
  m_pSplitterLocus->SetSashGravity(dGravity);
  m_pSplitterLocus->SetMinimumPaneSize(10);
  if(bSplitHorizontal)
  {
    m_pSplitter->SplitHorizontally(pPanelLocus,pPanelNotes,-180);
  }
  else
  {
    m_pSplitter->SplitVertically(pPanelLocus,pPanelNotes,
      -250);
  }
  m_pSplitter->SetSashGravity(1.0);
  m_pSplitter->SetMinimumPaneSize(10);

  pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(m_pSplitter,1, wxEXPAND);
  SetSizer(pSizer);
  pSizer->Layout();
}
wxPanel *CPanelLocusDetails::_CreateNotesPanel()
{
  wxPanel *pPanelNotes = new wxPanel(m_pSplitter,wxID_ANY);
//    pPanelNotes = new wxPanel(m_pSplitter,wxID_ANY);
  wxBoxSizer *pSizer;
  wxStaticText *pTextLabel = new wxStaticText(
    pPanelNotes,wxID_ANY,CLabels::NOTES);
  m_pTextNotes = new wxTextCtrl(
    pPanelNotes,wxID_ANY,wxEmptyString,
    wxDefaultPosition, wxDefaultSize,
    (wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_READONLY));
  nwxColorUtil::BackgroundParent(m_pTextNotes,true);
  pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(
    pTextLabel,0, (wxALL ^ wxBOTTOM) | wxALIGN_LEFT,ID_BORDER);
  pSizer->Add(
    m_pTextNotes,2, (wxALL ^ wxTOP) | wxEXPAND,ID_BORDER);
  if(m_bReadOnly && !m_bReadOnlyMayChange)
  {
    m_pTextNewNotes = NULL;
  }
  else
  {
    wxStaticText *pTextLabel2 = new wxStaticText(
      pPanelNotes,wxID_ANY,CLabels::ADDITIONAL_NOTES);
    m_pTextNewNotes = new wxTextCtrl(
      pPanelNotes,IDeditNotes,wxEmptyString,
      wxDefaultPosition, wxDefaultSize,
      (wxTE_MULTILINE | wxTE_DONTWRAP));
    pSizer->Add(pTextLabel2,0, 
      wxLEFT | wxRIGHT | wxALIGN_LEFT,ID_BORDER);
    pSizer->Add(m_pTextNewNotes,1,
      (wxALL ^ wxTOP) | wxEXPAND, ID_BORDER);
    if(m_bReadOnly) { m_pTextNewNotes->Enable(false); }
  }

  pPanelNotes->SetSizer(pSizer);
  return pPanelNotes;
}
bool CPanelLocusDetails::TransferDataToWindow()
{
   _InitData();
  wxString s = m_LocusEdit.FormatReviewAcceptance(m_HistTime);
  wxString sNotes = m_pSample->GetFile()->CheckLocusStatus(m_pLocus,m_HistTime);
  nwxString::Append(&s,sNotes);
  sNotes = m_LocusEdit.GetNotes(m_HistTime);
  nwxString::Append(&s,sNotes,CLabels::NOTES_AFTER_REVIEW);
  m_pTextNotes->SetValue(s);
  if(m_pTextNewNotes != NULL)
  {
    m_pTextNewNotes->Clear();
  }
  if(m_pGridLocus11 != NULL)
  {
    m_pGridLocus11->SetDateTime(m_HistTime,true);
  }
  else if(m_pGridLocusPeaks != NULL)
  {
    m_pGridLocusPeaks->SetDateTime(m_HistTime,true);
  }
  m_pGridAlerts->SetDateTime(m_HistTime,true);
  return true;
}

bool CPanelLocusDetails::Validate()
{
  bool bRtn = true;
  if( !m_bReadOnly && IsModified() )
  {
    wxString sMessage;
    if(IsNotesEmpty())
    {
      sMessage = "You haven't entered notes for this locus.\n"
        "Do you wish to continue?";
    }
    else if(!IsNotesModified())
    {
      sMessage = "You haven't updated notes for this locus.\n"
        "Do you wish to continue?";
    }
    else if(m_pPanelUser != NULL)
    {
      if(!m_pPanelUser->IsUserIDValid(&sMessage))
      {
        bRtn = false;
        mainApp::ShowError(sMessage,this);
        sMessage.Empty();
      }
    }
    if(!sMessage.IsEmpty())
    {
      bRtn = mainApp::Confirm(this,sMessage);
    }
  }
  return bRtn;
}

bool CPanelLocusDetails::IsModified()
{
  bool bRtn = IsAlertsModified() || IsLocusModified();
      // IsLocusModified() covers notes
  if(bRtn)
  {;}  // we are so done
  if( (!bRtn) && (m_pGridLocusPeaks != NULL) )
  {
    bRtn = m_pGridLocusPeaks->IsModified();
  }
  return bRtn;
}

bool CPanelLocusDetails::IsAlertsModified()
{
  bool bRtn = m_MsgEdit.IsModified(*m_pMsgs);
  return bRtn;
}
bool CPanelLocusDetails::IsLocusModified()
{
  bool bRtn = (m_LocusEdit != *m_pLocus);
  if((!bRtn) && (m_pTextNewNotes != NULL))
  {
    bRtn = IsNotesModified();
  }
  return bRtn;
}

bool CPanelLocusDetails::IsNotesModified()
{
  bool bRtn = false;
  if(m_pTextNewNotes != NULL)
  {
    wxString s = m_pTextNewNotes->GetValue();
    nwxString::Trim(&s);
    bRtn = !s.IsEmpty();
  }
  return bRtn;
}
bool CPanelLocusDetails::IsNotesEmpty()
{
  wxString s1 = m_LocusEdit.GetNotes();
  nwxString::Trim(&s1);
  bool bRtn = s1.IsEmpty() && !IsNotesModified();
  return bRtn;
}


bool CPanelLocusDetails::SetDateTime(const wxDateTime *pTime)
{
  bool bRtn = m_HistTime.SetDateTime(pTime);
  TransferDataToWindow();
  return bRtn;
}

void CPanelLocusDetails::UpdateFile(
  COARfile *pF, COARsample *pSample, const wxString &sUser)
{
  wxDateTime t;
  t.SetToCurrent();
  if(m_pGridLocusPeaks == NULL)
  {
    // this is a version 1.1 file
    pF->UpdateMessages(GetMessages(),t);
    pF->UpdateSampleLocus(pSample,m_LocusEdit,t);
  }
  else
  {
    COARlocus *pLocus = pSample->FindLocus(m_LocusEdit.GetName());
    wxString sErr;
    if(pLocus == NULL)
    {
      sErr = "CPanelLocusDetails::UpdateFile cannot find locus";
      wxASSERT_MSG(0,sErr);
      mainApp::LogMessage(sErr);
    }
    else if(!m_pGridLocusPeaks->TransferDataToSample(pSample))
    {
      sErr = "CPanelLocusDetails::UpdateFile Cannot Transfer Grid Data";
      wxASSERT_MSG(0,sErr);
      mainApp::LogMessage(sErr);
    }
    else
    {
      if(!sUser.IsEmpty())
      {
        CAppendReviewLocus x(pLocus);
        x.AppendReview(sUser); // don't need return value here
      }
      else if(m_pPanelUser != NULL)
      {
        // this will be obsolete
        CAppendReviewLocus x(pLocus);
        if(!m_pPanelUser->AddReview(&x,false))
        {
          sErr =
            "CPanelLocusDetails::UpdateFile - Cannot add review";
          wxASSERT_MSG(0,sErr);
          mainApp::LogMessage(sErr);
        }
      }
      _UpdateNotes(sUser);
      pLocus->UpdateNotes(*m_LocusEdit.GetNotesPtr(),t);
      pF->UpdateMessages(GetMessages(),t);
      pF->SetIsModified(true);
    }
  }
}
#if 1
void CPanelLocusDetails::_UpdateNotes(const wxString &_sUser)
{
  wxString sUser(_sUser);
  wxString sAppend;
  if(m_pTextNewNotes != NULL)
  {
    sAppend = m_pTextNewNotes->GetValue();
  }
  if(m_pPanelUser != NULL)
  {
    sUser = m_pPanelUser->GetValue();
  }
  m_LocusEdit.AppendNotes(sAppend,sUser);
}
#else
void CPanelLocusDetails::_UpdateNotes()
{
  wxString sNotes = m_LocusEdit.GetNotes(NULL);
  wxString s;
  wxString sNew;
  wxString sUser;
  wxDateTime dt;
  dt.SetToCurrent();
  if(m_pTextNewNotes != NULL)
  {
    s = m_pTextNewNotes->GetValue();
    nwxString::Trim(&s);
  }
  if(s.IsEmpty())
  {
    s = "<no new notes added>";
  }
  sNew = "Edited ";
  sNew += dt.Format(COARreview::TIME_FORMAT);
  if(m_pPanelUser != NULL)
  {
    sUser = m_pPanelUser->GetValue();
    nwxString::Trim(&sUser);
    if(!sUser.IsEmpty())
    {
      sNew.Append(" by ");
      sNew.Append(sUser);
    }
  }
  sNew.Append("\n");
  sNew.Append(s);
  if(!sNotes.IsEmpty())
  {
    sNotes.Append("\n");
  }
  sNotes.Append(sNew);
  m_LocusEdit.SetNotes(sNotes);
}
#endif
void CPanelLocusDetails::OnNotesChange(wxCommandEvent &)
{
  if(!m_bReadOnly)
  {
    _ProcessEvent();
  }
}
void CPanelLocusDetails::OnCellChange(wxGridEvent &)
{
  if(!m_bReadOnly)
  {
    _ProcessEvent();
  }
}

bool CPanelLocusDetails::_ProcessEvent()
{
  wxCommandEvent eSend(wxEVT_COMMAND_ENTER ,GetId());
  eSend.SetEventObject(this);
  return ProcessEvent(eSend);
}
BEGIN_EVENT_TABLE(CPanelLocusDetails,wxPanel)
EVT_TEXT(wxID_ANY,CPanelLocusDetails::OnNotesChange)
EVT_GRID_CMD_CELL_CHANGED(wxID_ANY,CPanelLocusDetails::OnCellChange)
END_EVENT_TABLE()
