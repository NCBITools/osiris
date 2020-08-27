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
*
*  FileName: CPrintOut.cpp
*  Author:   Douglas Hoffman
*
*/
#include <memory>

#undef ADJUST_ZOOM

#ifdef __WXMAC__
#include <wx/osx/printdlg.h>
#define ADJUST_ZOOM 1
#else
#define ADJUST_ZOOM 0
#endif
#include <wx/printdlg.h>
#include <wx/dc.h>

#include "CPrintOut.h"
#include "CPrintPreview.h"
#include "mainApp.h"
#include "CFramePlot.h"
#include "CFrameAnalysis.h"
#include "CParmOsiris.h"
#include "CDialogPrintSettings.h"
#include "wxIDS.h"
#ifdef TMP_DEBUG
#include "nwx/nwxStaticBitmap.h"
#endif

// static data and functions

wxPrintData *CPrintOut::g_printData = NULL;
wxPageSetupDialogData* CPrintOut::g_pageSetupData = NULL;
const int CPrintOut::MAX_PPI = 600;

wxPrintData *CPrintOut::GetPrintData()
{
  if (g_printData == NULL)
  {
    CParmOsirisGlobal pParm;
    g_printData = new wxPrintData();
    g_printData->SetOrientation(
      pParm->GetPrintPlotLandscape() ? wxLANDSCAPE : wxPORTRAIT
    );
    int nPaperID = pParm->GetPrintPlotPaperType();
    bool bSetPaperSize = true;
    if (nPaperID >= 0)
    {
      wxPaperSize nID = wxPaperSize(nPaperID);
      g_printData->SetPaperId(nID);
      bSetPaperSize = (nID == wxPAPER_NONE);
    }
    if(bSetPaperSize)
    {
      int nw = pParm->GetPrintPlotPaperWidth();
      int nh = pParm->GetPrintPlotPaperHeight();
      if (nw > 0 && nh > 0)
      {
        g_printData->SetPaperSize(wxSize(nw, nh));
      }
    }
  }
  return g_printData;
}
int CPrintOut::GetMinPage()
{
  return m_bPreview ? 0 : 1;
}
int CPrintOut::GetMaxPage()
{
  return 1;
}
bool CPrintOut::HasPage(int page)
{
  return (page <= GetMaxPage()) && (page >= GetMinPage());
}

void CPrintOut::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
  // TEST to see if it is responsible for printing selected pages.
#if 0
  wxPrintout::GetPageInfo(minPage, maxPage, selPageFrom, selPageTo);
#else
  *minPage = GetMinPage();
  *selPageFrom = 1;
  // page count for print preview may change upon changing settings
  // so maxPage is a high number.  For actual printing
  // the status window reports the number of pages being printed
  // so it needs to be accurate
  *maxPage = m_bPreview ? 32768 : GetMaxPage();
  *selPageTo = *maxPage;
#endif
}

wxPageSetupDialogData *CPrintOut::GetPageSetupData()
{
  if(g_pageSetupData == NULL)
  {
    CParmOsirisGlobal pParm;
    g_pageSetupData = new wxPageSetupDialogData(*GetPrintData());
    // set saved margins, to do save/retrieve paper ID or sice
    g_pageSetupData->SetMarginTopLeft(
      wxPoint(
        pParm->GetPrintPlotMarginLeft(),
        pParm->GetPrintPlotMarginTop()
      ));
    g_pageSetupData->SetMarginBottomRight(
        wxPoint(
          pParm->GetPrintPlotMarginRight(),
          pParm->GetPrintPlotMarginBottom()
        ));
    *g_pageSetupData = *GetPrintData();
  }
  return g_pageSetupData;
}
void CPrintOut::UpdatePageSetup()
{
  // update page setup in user osiris parameters
  CParmOsirisGlobal pParm;
  if (g_pageSetupData != NULL)
  {
    wxPoint pt;
    pt = g_pageSetupData->GetMarginTopLeft();
    pParm->SetPrintPlotMarginLeft((unsigned int)pt.x);
    pParm->SetPrintPlotMarginTop((unsigned int)pt.y);
    pt = g_pageSetupData->GetMarginBottomRight();
    pParm->SetPrintPlotMarginRight((unsigned int)pt.x);
    pParm->SetPrintPlotMarginBottom((unsigned int)pt.y);
  }
  if (g_printData != NULL)
  {
    wxSize szPaperSize(-1, -1);
    wxPaperSize nPaperID = g_printData->GetPaperId();
    pParm->SetPrintPlotPaperType(int(nPaperID));
    if (nPaperID == wxPAPER_NONE)
    {
      szPaperSize = g_printData->GetPaperSize();
    }
    pParm->SetPrintPlotPaperWidth(szPaperSize.GetWidth());
    pParm->SetPrintPlotPaperHeight(szPaperSize.GetHeight());
    pParm->SetPrintPlotLandscape(
      g_printData->GetOrientation() == wxLANDSCAPE);
  }
}
void CPrintOut::UpdatePageSetupData(wxPrintData *pPrintData, wxPageSetupDialogData *pSetupData)
{
  bool bUpdate = false;
  if (pPrintData != NULL)
  {
    bUpdate = true;
    *(GetPrintData()) = *pPrintData;
  }
  if (pSetupData != NULL)
  {
    bUpdate = true;
    *(GetPageSetupData()) = *pSetupData;
  }
  if (bUpdate)
  {
    UpdatePageSetup(); // save user settings
  }
}

void CPrintOut::DoPageSetup(wxFrame *pParent)
{
  wxPageSetupDialogData *pSetupData = GetPageSetupData();
  wxPrintData *pPrintData = GetPrintData();
  *pSetupData = *pPrintData;

  wxPageSetupDialog pageSetupDialog(pParent, pSetupData);
  pageSetupDialog.ShowModal();
  UpdatePageSetupData(
    &pageSetupDialog.GetPageSetupDialogData().GetPrintData(),
    &pageSetupDialog.GetPageSetupDialogData()
  );

  mainApp::Ping(PING_EVENT, wxT("PrintPageSetup"));
}

#ifdef __WXMAC__
void CPrintOut::DoPageMargins(wxFrame *parent)
{
  wxPageSetupDialogData *pSetupData = GetPageSetupData();
  wxPrintData *pPrintData = GetPrintData();
  *pSetupData = *pPrintData;

  wxMacPageMarginsDialog pageMarginsDialog(parent, g_pageSetupData);
  pageMarginsDialog.ShowModal();

  UpdatePageSetupData(
    &pageMarginsDialog.GetPageSetupDialogData().GetPrintData(),
    &pageMarginsDialog.GetPageSetupDialogData()
  );
}
#endif

void CPrintOut::_DoPrint(CPrintOut *pPrintout, const wxString &sPingType)
{
  wxPrintDialogData printDialogData(*GetPrintData());

  wxPrinter printer(&printDialogData);
  wxString sStatus;
  if (printer.Print(pPrintout->GetParent(), pPrintout, true /*prompt*/))
  {
    // OK
    UpdatePageSetupData(
      &(printer.GetPrintDialogData().GetPrintData()),
      NULL
    );
    sStatus = wxT("OK");
  }
  else if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
  {
    // error
    mainApp::LogMessage(wxT("Printer error"));
    sStatus = wxT("ERROR");
  }
  else
  {
    // printout canceled
    sStatus = wxT("CANCEL");
  }
  mainApp::Ping2(PING_EVENT, sPingType, wxT("Status"), sStatus);
}

void CPrintOut::_DoPrintPreview(
  CPrintOut *pPreview,
  CPrintOut *pPrint,
  const wxString &sTitle,
  const wxString &sPingPreview,
  const wxString &sPingPrint)
{
  wxPrintDialogData printDialogData(*GetPrintData());
  CPrintPreview *preview =
    new CPrintPreview(
      sPingPrint,
      pPreview,
      pPrint,
      &printDialogData);
  wxString sStatus;
  CPrintPreviewFrame *pFrame = NULL;
  if (!preview->IsOk())
  {
    delete preview;
    mainApp::ShowError(
      wxT("There was a problem with print preview.\nPlease check your printer setup."),
      pPreview->GetParent());
    sStatus = wxT("ERROR");
  }
  else
  {
    sStatus = wxT("OK");
    pFrame =
      new CPrintPreviewFrame(preview, pPreview->GetParent(), sTitle);
  }
  mainApp::Ping2(PING_EVENT, sPingPreview, wxT("Status"), sStatus);
  if (pFrame != NULL)
  {
    // "Ping" before show
    pFrame->Show();
  }
}




// end static functions

CPrintOut::~CPrintOut()
{
}


void CPrintOut::_setupPageBitmap(wxDC *pdc)
{
  if (!IsPreview())
  {
    wxRect rectPixels(GetPaperRectPixels());
    FitThisSizeToPaper(rectPixels.GetSize());
  }
  wxRect rectPage(GetLogicalPageMarginsRect(*GetPageSetupData()));
  _resInput res(pdc->GetPPI(), rectPage);
  if (res != m_resInput)
  {
    // compute everything
    wxSize szPPI = res.m_ppi;
    wxRect rectFit = res.m_logicalPage;
    double dPPIscale = 1.0, dScalePixel = 1.0;
    int nMaxPPI = MAX_PPI;
    enum
    {
      SCALE_NONE,
      SCALE_X,
      SCALE_Y
    } nScale = SCALE_NONE;
    int nPPIx, nPPIy, nMinPPI, nUsePPI, nX, nY;
    bool bFit = false;

    m_resInput = res;
    nPPIx = szPPI.GetWidth();
    nPPIy = szPPI.GetHeight();

#if ADJUST_ZOOM
    CPrintPreview *pp = IsPrintPreview() ? (CPrintPreview *)GetPreview() : NULL;
    if(pp != NULL)
    {
      int nZoom = pp->GetZoom();
      if(nZoom != 100)
      {
  double dMult = double(nZoom) * 0.01;
  nPPIx = int(nPPIx * dMult);
  nPPIy = int(nPPIy * dMult);
      }
    }
#endif

    if (nPPIx == nPPIy)
    {
      nMinPPI = nPPIx;
    }
    else if (nPPIx < nPPIy)
    {
      // pixels are not square, adjust resolution
      nMinPPI = nPPIx;
      nScale = SCALE_Y;
      dScalePixel = double(nPPIx) / double(nPPIy);
    }
    else
    {
      nMinPPI = nPPIy;
      nScale = SCALE_X;
      dScalePixel = double(nPPIy) / double(nPPIx);
    }
    if (IsPreview())
    {
      int nx, ny;

      GetPPIScreen(&nx, &ny);
      nMaxPPI = (nx < ny) ? nx : ny;
      if (nMaxPPI < 48)
      {
        nMaxPPI = 48;
      }
    }
#ifdef TMP_DEBUG
    else
    {
      if (!m_nLoggedPrinterResolution)
      {
        wxString sLOG = wxString::Format(
          wxT("Printer resolution %d x %d, size %d x %d"),
          szPPI.GetX(),
          szPPI.GetY(),
          rectPage.GetWidth(),
          rectPage.GetHeight()
        );
        mainApp::LogMessage(sLOG);
        m_nLoggedPrinterResolution++;
      }
    }
#endif

    if (nMinPPI > nMaxPPI)
    {
      bFit = true;
      nUsePPI = nMaxPPI;
      dPPIscale = nMaxPPI / float(nMinPPI);
      nX = int(rectFit.GetWidth() * dPPIscale);
      nY = int(rectFit.GetHeight() * dPPIscale);
    }
    else
    {
      nUsePPI = nMinPPI;
      nX = rectFit.GetWidth();
      nY = rectFit.GetHeight();
    }
    switch (nScale)
    {
    case SCALE_NONE:
      break;
    case SCALE_X:
      nX = int(dScalePixel * nX);
      bFit = true;
      break;
    case SCALE_Y:
      nY = int(dScalePixel * nY);
      bFit = true;
      break;
    }
    m_resOutput.m_bFit = bFit;
    m_resOutput.m_DPI = nUsePPI;
    m_resOutput.m_nWidth = nX;
    m_resOutput.m_nHeight = nY;
  }
  bool bAdjust = m_resOutput.m_bFit;
  int nWidth = m_resOutput.m_nWidth;
  int nHeight = m_resOutput.m_nHeight;
  if (bAdjust)
  {
#ifdef TMP_DEBUG
    wxString sLOG = wxString::Format(
      wxT("Bitmap size size %d x %d"),
      nWidth, nHeight);
    mainApp::LogMessage(sLOG);
#endif
    FitThisSizeToPageMargins(wxSize(nWidth, nHeight), *GetPageSetupData());
  }
}

#ifdef TMP_DEBUG
void CPrintOut::DebugBitmap(wxBitmap *pBitmap, int nPage)
{
  wxString sFile = wxString::Format(wxT("%s_%lx_%d.png"),
      IsPreview() ? "S" : "P",  // screen (preview) or printout
      (long) this, nPage);
  mainApp::DumpBitmap(pBitmap, sFile);
}
#endif

IMPLEMENT_ABSTRACT_CLASS(CPrintOut, wxPrintout)
