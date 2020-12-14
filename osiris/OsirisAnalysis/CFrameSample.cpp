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
*  OSIRIS is a desktop tool working on your computer with your own data.
*  Your sample profile data is processed on your computer and is not sent
*  over the internet.
*
*  For quality monitoring, OSIRIS sends some information about usage
*  statistics  back to NCBI.  This information is limited to use of the
*  tool, without any sample, profile or batch data that would reveal the
*  context of your analysis.  For more details and instructions on opting
*  out, see the Privacy Information section of the OSIRIS User's Guide.
*
* ===========================================================================
*
*  FileName: CFrameSample.cpp
*  Author:   Douglas Hoffman
*
*/
#include "CFrameSample.h"
#include "COARfile.h"
#include "CHistoryTime.h"
#include "CNotebookEditSample.h"
#include "mainFrame.h"
#include "CFrameAnalysis.h"
#include "CMenuSample.h"
#include "CDialogCellHistory.h"
#include "CDialogApprove.h"
#include "CDialogWarnHistory.h"
#include "CPageEditSample.h"
#include "CSiteSettings.h"
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include "nwx/nwxLog.h"
#include "nwx/nwxString.h"
#include "COsirisIcon.h"
#include "CPanelSampleTitle.h"
#include "CFramePlot.h"

IMPLEMENT_ABSTRACT_CLASS(CFrameSample,CMDIFrame)

#define PING_WINDOW_TYPE "FrameSample"
#define LINE_SPACER "\n    "

#ifdef __WXMSW__
#define INITSIZE(sz) wxSize(sz.x < 0 ? -1 : sz.x - 1 ,sz.y)
#else
#define INITSIZE(sz) sz
#endif

CFrameSample::CFrameSample(
  CFrameAnalysis *pCreator,
  mainFrame *parent,
  const wxSize &sz,
  COARfile *pFile,
  COARsample *pSample,
  int nSelectAlerts,
  const wxString &sSelectLocus) : 
    CMDIFrame(parent,wxID_ANY,wxEmptyString,wxDefaultPosition,INITSIZE(sz)),
    m_sz(sz),
    m_Hist(NULL),
    m_pNoteBook(NULL),
    m_pCreator(pCreator),
    m_pParent(parent),
    m_pOARfile(pFile),
    m_pSample(pSample),
    m_nTileWithGraph(0),
    m_bTitleMod(false)

    //,m_bFirstShow(true)
{
  wxString s = wxPanelNameStr;
  mainApp::Ping2(PING_EVENT, PING_WINDOW_OPEN PING_WINDOW_TYPE, PING_WINDOW_NUMBER, GetFrameNumber());
  SetupTitle(true);
  wxPanel *pPanel = new wxPanel(this);
  m_pNoteBook = new CNotebookEditSample(m_pOARfile,m_pSample,pPanel,wxID_ANY,NULL);
  if(!sSelectLocus.IsEmpty())
  {
    SelectLocus(sSelectLocus);
  }
  else if(nSelectAlerts >= 0)
  {
    SelectAlerts(nSelectAlerts);
  }
  m_pToolbar = new CToolbarSample(this,pPanel);
  m_pParent->InsertWindow(this,m_pOARfile);
  m_pMenuBar = new CMenuBarSample();
  SetMenuBar(m_pMenuBar);
  bool bOverride =
    m_pOARfile->GetLabSettings().GetReviewerAllowUserOverride();
  m_pTitle = new CPanelSampleTitle(this, m_pSample,
    GetDefaultUserID(), !bOverride);
  SetupMenuItems(); 
  CParmOsirisGlobal parm;
//  bool b = parm->GetHideSampleToolbar();
//  _ShowToolbar(!b);
  wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(m_pNoteBook,1,wxEXPAND,0);
  pSizer->Add(m_pToolbar,0,wxEXPAND);
  pPanel->SetSizer(pSizer);
  pSizer = new wxBoxSizer(wxVERTICAL);
  pSizer->Add(m_pTitle,0,wxEXPAND);
  pSizer->Add(pPanel,1,wxEXPAND | wxALL,0);

  SetSizer(pSizer);
  TransferDataToWindow();
  COsirisIcon x;
  SetIcon(x);
  RE_RENDER;
}
CFrameSample::~CFrameSample() 
{
  mainApp::Ping2(PING_EVENT, PING_WINDOW_CLOSE PING_WINDOW_TYPE, PING_WINDOW_NUMBER, GetFrameNumber());
}

void CFrameSample::SelectLocus(const wxString &sLocus)
{
  m_pNoteBook->SelectLocus(sLocus);
}
void CFrameSample::SelectAlerts(int nAlertType)
{
  m_pNoteBook->SelectAlerts(nAlertType);
}

int CFrameSample::GetType()
{
  return FRAME_SAMPLE;
}
wxMenu *CFrameSample::GetMenu()
{
  return m_pMenuBar->GetMenu();
}
wxString CFrameSample::GetDisplayedSampleName()
{
  CParmOsiris *pParmGlobal = CParmOsiris::GetGlobal();
  int nSampleNameLabel = pParmGlobal->GetTableShowSampleDisplayNames();
  wxString sName = (nSampleNameLabel == IDmenuDisplayNameSample)
    ? m_pSample->GetSampleName() //  sample name
    : m_pSample->GetName();      //  sample file name (plt, fsa, hid)
  return sName;
}
void CFrameSample::SetupTitle(bool bForce)
{
  bool bMod = (m_pNoteBook != NULL) && m_pNoteBook->IsModified();
  if(bForce || (bMod != m_bTitleMod))
  {
    wxString sName = GetDisplayedSampleName();
    m_bTitleMod = bMod;
    sName.Append(wxT("; "));
    wxFileName fn(m_pOARfile->GetFileName());
    sName.Append(fn.GetFullName());
    const CParmOsiris &parm(m_pOARfile->GetParameters());
    wxString sTitle = mainApp::FormatWindowTitle(wxT("Sample"),sName,bMod,&parm,NULL);
    SetTitle(sTitle);
  }
}

void CFrameSample::OnTimer(wxTimerEvent &) 
{
  if(m_nTileWithGraph > 0)
  {
    --m_nTileWithGraph;
    if(!m_nTileWithGraph)
    {
      _TileWithGraph();
    }
  }
}
bool CFrameSample::TransferDataToWindow()
{
  //  it seems that this isn't being used
  SetupMenuItems();
  return m_pNoteBook->TransferDataToWindow();
}

bool CFrameSample::CheckApplyNoNotes(int nPanel)
{
  //  returns false if no new notes have been entered
  //  and the user replies "No" to continue processing
  //  when prompted
  const CSiteSettings *pSettings = nwxGET_GLOBAL(CSiteSettings);
  bool bWarnOnNotes = pSettings->GetBoolValue("WarnEmptyNotes",true);
  bool bRtn = true;
  if(bWarnOnNotes)
  {
    std::vector<wxString> vsPages;
    size_t nStart = 0;
    size_t nEnd;
    CPageEditSample *pPage;
    if(nPanel < 0)
    {
      nStart = 0;
      nEnd = m_pNoteBook->GetPanelCount();
      vsPages.reserve(nEnd);
    }
    else
    {
      nEnd = nStart = (size_t)nPanel;
      ++nEnd;
    }
    for(size_t i = nStart; i < nEnd; ++i)
    {
      pPage = m_pNoteBook->GetPanel(i);
      if((pPage != NULL) && 
          pPage->NeedsApply() &&
          pPage->IsNewNotesEmpty())
      {
        vsPages.push_back(pPage->GetPageLabel());
      }
    }
    size_t nSize = vsPages.size();
    if(nSize)
    {
      wxString sMessage;
      wxString sList;
      nwxString::Join(vsPages,&sList,LINE_SPACER);
      sMessage.Append(wxT("You haven't entered notes for"));
      sMessage.Append(LINE_SPACER);
      sMessage.Append(sList);
      sMessage.Append(wxT("\n\nDo you wish to continue to apply changes?"));

      wxMessageDialog dlg(this,sMessage,wxT("Warning"),wxYES_NO | wxNO_DEFAULT);
      bRtn = (dlg.ShowModal() == wxID_YES);
    }
  }
  return bRtn;
}

void CFrameSample::_ApplyAll()
{
  bool bContinue = CheckApplyNoNotes();
  size_t nCount = 
    bContinue ? m_pNoteBook->GetPanelCount() : 0;
  CPageEditSample *pPage;
  bool bRefresh = false;
  for(size_t i = 0; i < nCount; ++i)
  {
    pPage = m_pNoteBook->GetPanel(i);
    if((pPage != NULL) && pPage->NeedsApply() && pPage->DoApply())
    {
      bRefresh = true;
    }
  }
  if(bRefresh)
  {
    InitiateRepaintData();
  }
}
void CFrameSample::_Accept()
{
  CPageEditSample *pPage = m_pNoteBook->GetCurrentPanel();
  if((pPage != NULL) && pPage->DoAccept())
  {
    InitiateRepaintData();
  }
}
void CFrameSample::_Approve()
{
  const wxString &sLocus = m_pNoteBook->GetCurrentLocus();
  int nSelected = m_pNoteBook->GetSelection();
  int n = -1;
  if(nSelected < SA_WINDOW_COUNT)
  {
    wxSize sz(SIZE_LOCUS_HISTORY);
    CDialogApprove dlg(
      nSelected,
      m_pOARfile,
      m_pSample,
      false,
      this,
      wxID_ANY,sz);
    dlg.SetUserID(GetUserID());
    n = dlg.ShowModal();
  }
  else
  {
    wxASSERT_MSG(!sLocus.IsEmpty(),wxT("CFrameSample::_Approve() locus not specified"));
    wxSize sz(SIZE_EDIT_LOCUS);
    const COARmessages *pMsg = m_pOARfile->GetMessages();
    COARlocus *pLocus = m_pSample->FindLocus(sLocus);
    const COARchannel *pChannel =
          m_pOARfile->GetChannelFromLocus(sLocus);
    int nChannel =
          (pChannel == NULL) ? -1 : pChannel->GetChannelNr();
    CDialogApprove dlg(
      *m_pSample,
      nChannel,
      pLocus,
      *pMsg,
      *(m_pOARfile->GetHistory()),
      false,
      false,
      this,
      wxID_ANY,
      sz);
    dlg.SetUserID(GetUserID());
    n = dlg.ShowModal();
  }
  if(n == wxID_OK)
  {
    InitiateRepaintData();
  }   
}
void CFrameSample::_ToggleDisabled()
{
  CPageEditSample *pPage = m_pNoteBook->GetPanel(SA_NDX_SAMPLE);
  if( (pPage != NULL) && (pPage->DoToggleEnabled()) )
  {
    InitiateRepaintData();
  }
}
void CFrameSample::_ShowTable()
{
  m_pCreator->RaiseWindow();
  const wxString &sLocus = m_pNoteBook->GetCurrentLocus();
  wxString sName = m_pSample->GetName();
  bool b = false;
  if(sLocus.IsEmpty())
  {
    int n = m_pNoteBook->GetSelection();
    b = m_pCreator->SelectSampleType(sName,n);
  }
  else
  {
    b = m_pCreator->SelectSampleLocus(sName, sLocus);
  }
  if(!b)
  {
    nwxLog::LogMessage("CFrameSample::_ShowTable() could not locate row/col");
  }
}
bool CFrameSample::MenuEvent(wxCommandEvent &e)
{
  int nID = e.GetId();
  bool bRtn = true;
  switch(nID)
  {
  case IDmenuSampleTile:
#ifdef __WXMAC__
    if(!CheckCannotTile(this,true))
#endif
    {
      m_nTileWithGraph++;
    }
    break;
  case IDmenuDisplayGraph:
    _OpenGraphic();
    break;
  case IDmenuTable:
    _ShowTable();
    break;
  case IDmenuHistory:
    _History();
    break;
  case IDSampleApply:
    _ApplyAll();
    break;
  case IDSampleAccept:
    _Accept();
    break;
  case IDSampleApprove:
    _Approve();
    break;
  case IDmenuDisableSample:
    _ToggleDisabled();
    break;
  default:
    break;
  }
  return bRtn;
}

void CFrameSample::InitiateRepaintData()
{
  bool bCheckHistory = !m_pCreator->HistoryIsCurrent();
  if (!bCheckHistory)
  {
    CFramePlot *pPlot = m_pParent->FindPlotWindowBySample(m_pSample);
    bCheckHistory = (pPlot != NULL) && !pPlot->HistoryIsCurrent();
  }
  if (bCheckHistory)
  {
    CDialogWarnHistory::Continue(this, false);
  }
  m_pCreator->RepaintAllData(m_pSample);
}
void CFrameSample::RepaintData()
{
  m_pNoteBook->TransferDataToWindow();
  SetupMenuItems();
  SetupTitle(true);
}
const wxString &CFrameSample::GetUserID()
{
  return m_pTitle->GetUserID();
}
const wxString &CFrameSample::GetDefaultUserID()
{
  if(m_sUserID.IsEmpty())
  {
    if(CanOverrideUserID())
    {
      CParmOsiris *parm = CParmOsiris::GetGlobal();
      m_sUserID = parm->GetCMFuserID();
    }
    if(m_sUserID.IsEmpty())
    {
      m_sUserID = wxGetUserId();
    }
  }
  return m_sUserID;
}
bool CFrameSample::CanOverrideUserID()
{
  return m_pOARfile->GetReviewerAllowUserOverride();
}
void CFrameSample::SaveUserID()
{
  if( CanOverrideUserID() && !m_sUserID.IsEmpty() )
  {
    CParmOsiris *parm = CParmOsiris::GetGlobal();
    parm->SetCMFuserID(m_sUserID);
  }
}

void CFrameSample::_TileWithGraph()
{
  _OpenGraphic(true);
  const wxString &sFileName = m_pSample->GetPlotFileName();
  CMDIFrame *pTempFrame = m_pParent->FindWindowByName(sFileName);
  CFramePlot *pFrame = pTempFrame == NULL ? NULL 
    : wxDynamicCast(pTempFrame,CFramePlot);
  if(pFrame == NULL)
  {
    mainApp::ShowError(wxT("Cannot find plot window"),this);
  }
#ifdef __WXMAC__
  else if(CheckCannotTile(pFrame))
  {
    m_pCreator->Raise();
  }
#endif
  else if(!m_nTileWithGraph)
  {
    bool bDelay = false;
    if(CheckRestore()) 
    {
      bDelay = true; 
    }
    else if(!IsShown()) 
    {
      Show(true);
      bDelay = true; 
    }
    if(pFrame->CheckRestore()) 
    {
      bDelay = true; 
    }
    else if(!pFrame->IsShown()) 
    {
      pFrame->Show(true);
      bDelay = true;
    }
    if(bDelay)
    {
      m_nTileWithGraph++;
    }
    else
    {
      CBatchPlot xxx(pFrame);
      m_pParent->TileTwoWindows(this,pFrame);
    }
  }
}

void CFrameSample::_OpenGraphic(bool bNoChange)
{
  // bNoChange - if true and window exists, do not change view
  const wxString &sLocus = m_pNoteBook->GetCurrentLocus(true);
  const wxString &sFileName = m_pSample->GetPlotFileName();
  m_pParent->OpenFile(sFileName,sLocus,m_pOARfile,bNoChange);
}
void CFrameSample::_History()
{
  const wxString &sLocus = m_pNoteBook->GetCurrentLocus();
  int nSelected = m_pNoteBook->GetSelection();
  CDialogCellHistory::ShowHistory(
    this,m_pOARfile, m_pSample,sLocus,nSelected);
}

bool CFrameSample::IsModified(int *pnFirstPage)
{
  return m_pNoteBook->IsModified(pnFirstPage);
}
void CFrameSample::Select(int n)
{
  m_pNoteBook->Select(n);
}
bool CFrameSample::Destroy()
{
  m_pCreator->RemoveSample(m_pSample,this);
  return SUPER::Destroy();
}
void CFrameSample::FormatCloseWarning(const std::vector<wxString> &vsSamples, wxString *ps)
{
  wxString sName;
  const wxChar *ps1;
  const wxChar *ps2 = wxT(
    ",\nhave not been applied and will be lost.\n\nDo you wish to close this window?"
    );  // 76
  size_t nSize = vsSamples.size();
  ps->Clear();
  if(nSize)
  {
    if(nSize == 1)
    {
      ps1 = wxT("Modifications to this sample,");
      sName = vsSamples.at(0);
    }
    else
    {
      // nSize > 1
      ps1 = wxT("Modification to these samples:");  // 30
      nwxString::Join(vsSamples,&sName,LINE_SPACER);
      ps2++; // skip over comma
    }
    ps->reserve(128 + sName.Len());
    ps->Append(ps1);
    ps->Append(LINE_SPACER);
    ps->Append(sName);
    ps->Append(ps2);
  }
}

void CFrameSample::OnClose(wxCloseEvent &e)
{
  bool bDestroy = true;
  if(e.CanVeto())
  {
    int nSelect;
    if(m_pNoteBook->IsModified(&nSelect))
    {
      wxString sWarning;
      std::vector<wxString> vs;
      vs.push_back(GetDisplayedSampleName());
      FormatCloseWarning(vs,&sWarning);
      wxMessageDialog dlg(this,sWarning,wxT("Warning"),wxYES_NO | wxNO_DEFAULT);
      int n = dlg.ShowModal();
      if(n != wxID_YES)
      {
        bDestroy = false;
        e.Veto(true);
        m_pNoteBook->Select(nSelect);
        RaiseWindow();
      }
    }
  }
  if(bDestroy)
  {
    e.Skip();
  }
}

void CFrameSample::UpdateSizeHack(bool bForce)
{
  //  this frame does not render properly 
  //  until it is resized, so, here goes
  wxSize szBeenHere(-5,-5);
  bool bDoIt = (m_sz != szBeenHere);
  if(bDoIt && !bForce)
  {
    bDoIt = IsShown() &&
      !(IsMaximized() || IsIconized());
  }
  if(!bDoIt) {}
  else if(m_sz == wxDefaultSize) 
  {
    m_sz = GetSize();
    m_sz.x--;
  }
  else if(m_sz == GetSize())
  {
    if(bForce)
    {
      m_sz.x--;
    }
    else
    {
      bDoIt = false;
    }
  }
  if(bDoIt)
  {
    SetSize(m_sz);
#ifdef __WXDEBUG__
    wxString s;
    s.Format(wxT("size %d x %d"),m_sz.x,m_sz.y);
    nwxLog_I_WAS_HERE_MSG(s);
#endif
    m_sz = szBeenHere;
  }
}

void CFrameSample::SetupMenuItems()
{
  CPageEditSample *pPage = m_pNoteBook->GetCurrentPanel();
  if(pPage != NULL)
  {
    int nItem = m_pNoteBook->GetSelection();
    bool bSampleEnabled = m_pSample->IsEnabled();
    bool bDirPanel = (nItem == SA_NDX_DIR);
    bool bEnabledOrDir = bSampleEnabled || bDirPanel;
    _EnableItem(IDmenuHistory,pPage->HasHistory());
    _EnableItem(IDSampleAccept, bEnabledOrDir && pPage->NeedsAcceptance());
    _EnableItem(IDSampleApprove, bEnabledOrDir && pPage->NeedsReview());
    _EnableItem(IDSampleApply, bSampleEnabled && m_pNoteBook->IsModified());
    _EnableItem(IDmenuDisableSample, 
      m_pSample->CanDisableSample() && !bDirPanel);
    _SetSampleEnabled(bSampleEnabled);
  }
}

IMPLEMENT_PERSISTENT_SIZE(CFrameSample)

BEGIN_EVENT_TABLE(CFrameSample,CMDIFrame)
EVT_PERSISTENT_SIZE(CFrameSample)
EVT_CLOSE(CFrameSample::OnClose)
END_EVENT_TABLE()



