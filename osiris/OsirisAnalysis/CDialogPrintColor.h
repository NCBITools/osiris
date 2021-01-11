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
*
*  FileName: CDialogPrintColor.h
*  Author:   Douglas Hoffman
*
*/
#ifndef __C_DIALOG_PRINT_COLOR_H__
#define __C_DIALOG_PRINT_COLOR_H__

#include <wx/dialog.h>
#include <wx/colour.h>
#include <wx/hyperlink.h>
#include "nwx/PersistentSize.h"

class nwxTextCtrlInteger;
class wxSizer;

class CDialogPrintColor : public wxDialog
{
public:
  CDialogPrintColor(wxWindow *parent);
  virtual ~CDialogPrintColor();
  virtual bool TransferDataFromWindow();
  virtual bool TransferDataToWindow();
private:
  void _PrintTestPage();
  void _OnPrintLink(wxHyperlinkEvent &);
  void _OnPrintButton(wxCommandEvent &);
  void _ColorRow(
    wxPanel *pPanel,
    wxSizer *pSizer, 
    const wxString &sLabel,
    int nValue,
    const wxColour &color,
    nwxTextCtrlInteger **ppCtrl);
  nwxTextCtrlInteger *m_pTextRed;
  nwxTextCtrlInteger *m_pTextGreen;
  nwxTextCtrlInteger *m_pTextBlue;
  nwxTextCtrlInteger *m_pTextOrange;
  nwxTextCtrlInteger *m_pTextYellow;
  nwxTextCtrlInteger *m_pTextPurple;
  nwxTextCtrlInteger *m_pTextGray;
  DECLARE_PERSISTENT_POSITION
  DECLARE_EVENT_TABLE()
};

#endif