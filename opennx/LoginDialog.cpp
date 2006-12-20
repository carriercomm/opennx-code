// $Id$
//
// Copyright (C) 2006 The OpenNX Team
// Author: Fritz Elfert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "LoginDialog.h"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/config.h>
#include <wx/dir.h>
#include <wx/xml/xml.h>

////@begin includes
////@end includes

#include "LoginDialog.h"
#include "MyXmlConfig.h"
#include "SessionProperties.h"
#include "MySession.h"
#include "Icon.h"

////@begin XPM images
////@end XPM images

static wxString MYTRACETAG(wxFileName::FileName(wxT(__FILE__)).GetName());

/*!
 * LoginDialog type definition
 */

IMPLEMENT_CLASS( LoginDialog, wxDialog )

    /*!
     * LoginDialog event table definition
     */

BEGIN_EVENT_TABLE( LoginDialog, wxDialog )

    ////@begin LoginDialog event table entries
    EVT_COMBOBOX( XRCID("ID_COMBOBOX_SESSION"), LoginDialog::OnComboboxSessionSelected )

    EVT_CHECKBOX( XRCID("ID_CHECKBOX_SMARTCARD"), LoginDialog::OnCheckboxSmartcardClick )

    EVT_CHECKBOX( XRCID("ID_CHECKBOX_GUESTLOGIN"), LoginDialog::OnCheckboxGuestloginClick )

    EVT_BUTTON( XRCID("ID_BUTTON_CONFIGURE"), LoginDialog::OnButtonConfigureClick )

    EVT_BUTTON( wxID_OK, LoginDialog::OnOkClick )

    ////@end LoginDialog event table entries

END_EVENT_TABLE()

    /*!
     * LoginDialog constructors
     */

    LoginDialog::LoginDialog( )
: m_pCurrentCfg(NULL)
{
}

    LoginDialog::LoginDialog( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: m_pCurrentCfg(NULL)
{
    Create(parent, id, caption, pos, size, style);
}

LoginDialog::~LoginDialog()
{
    if (m_pCurrentCfg)
        delete m_pCurrentCfg;
}

void LoginDialog::ReadConfigDirectory()
{
    wxString cfgdir;
    wxConfigBase::Get()->Read(wxT("Config/UserNxDir"), &cfgdir);
    cfgdir = cfgdir + wxFileName::GetPathSeparator() + wxT("config");
    m_aConfigFiles.Empty();
    m_aSessionNames.Empty();

    wxDir::GetAllFiles(cfgdir, &m_aConfigFiles, wxT("*.nxs"), wxDIR_FILES);
    size_t i;
    if (m_pCurrentCfg)
        delete m_pCurrentCfg;
    m_pCurrentCfg = NULL;
    for (i = 0; i < m_aConfigFiles.GetCount(); i++) {
        MyXmlConfig cfg(m_aConfigFiles[i]);
        if (cfg.IsValid()) {
            if (m_sSessionName.IsEmpty())
                m_sSessionName = cfg.sGetName();
            m_aSessionNames.Add(cfg.sGetName());
            if (cfg.sGetName() == m_sSessionName) {
                m_pCurrentCfg = new MyXmlConfig(m_aConfigFiles[i]);
                m_bGuestLogin = cfg.bGetGuestMode();
                m_sUsername = cfg.sGetUsername();
                m_sPassword = cfg.sGetPassword();
#ifdef ENABLE_SMARTCARD
                m_bUseSmartCard = cfg.bGetUseSmartCard();
#else
                m_bUseSmartCard = false;
#endif
            }
        }
    }
    if (m_aSessionNames.GetCount() == 0) {
        m_aSessionNames.Add(wxT(""));
        m_sSessionName = wxT("");
    } else {
        if (!m_pCurrentCfg) {
            m_sSessionName = wxT("");
            ReadConfigDirectory();
        }
    }
}

/*!
 * LoginDialog creator
 */

bool LoginDialog::Create( wxWindow* parent, wxWindowID WXUNUSED(id), const wxString& WXUNUSED(caption), const wxPoint& WXUNUSED(pos), const wxSize& WXUNUSED(size), long WXUNUSED(style) )
{
    ////@begin LoginDialog member initialisation
    m_sTmpUsername = wxT("");
    m_sTmpPassword = wxT("");
    m_pCtrlUsername = NULL;
    m_pCtrlPassword = NULL;
    m_pCtrlSessionName = NULL;
    m_pCtrlUseSmartCard = NULL;
    m_pCtrlGuestLogin = NULL;
    ////@end LoginDialog member initialisation

    ////@begin LoginDialog creation
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    SetParent(parent);
    CreateControls();
    SetIcon(GetIconResource(wxT("res/nx.png")));
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
    ////@end LoginDialog creation
    return TRUE;
}

/*!
 * Control creation for LoginDialog
 */

void LoginDialog::CreateControls()
{

    ReadConfigDirectory();

    ////@begin LoginDialog content construction
    if (!wxXmlResource::Get()->LoadDialog(this, GetParent(), _T("ID_DIALOG_LOGIN")))
        wxLogError(wxT("Missing wxXmlResource::Get()->Load() in OnInit()?"));
    m_pCtrlUsername = XRCCTRL(*this, "ID_TEXTCTRL_USERNAME", wxTextCtrl);
    m_pCtrlPassword = XRCCTRL(*this, "ID_TEXTCTRL_PASSWORD", wxTextCtrl);
    m_pCtrlSessionName = XRCCTRL(*this, "ID_COMBOBOX_SESSION", wxComboBox);
    m_pCtrlUseSmartCard = XRCCTRL(*this, "ID_CHECKBOX_SMARTCARD", wxCheckBox);
    m_pCtrlGuestLogin = XRCCTRL(*this, "ID_CHECKBOX_GUESTLOGIN", wxCheckBox);
    // Set validators
    if (FindWindow(XRCID("ID_TEXTCTRL_USERNAME")))
        FindWindow(XRCID("ID_TEXTCTRL_USERNAME"))->SetValidator( wxGenericValidator(& m_sUsername) );
    if (FindWindow(XRCID("ID_TEXTCTRL_PASSWORD")))
        FindWindow(XRCID("ID_TEXTCTRL_PASSWORD"))->SetValidator( wxGenericValidator(& m_sPassword) );
    if (FindWindow(XRCID("ID_COMBOBOX_SESSION")))
        FindWindow(XRCID("ID_COMBOBOX_SESSION"))->SetValidator( wxGenericValidator(& m_sSessionName) );
    if (FindWindow(XRCID("ID_CHECKBOX_SMARTCARD")))
        FindWindow(XRCID("ID_CHECKBOX_SMARTCARD"))->SetValidator( wxGenericValidator(& m_bUseSmartCard) );
    if (FindWindow(XRCID("ID_CHECKBOX_GUESTLOGIN")))
        FindWindow(XRCID("ID_CHECKBOX_GUESTLOGIN"))->SetValidator( wxGenericValidator(& m_bGuestLogin) );
    ////@end LoginDialog content construction

    // Create custom windows not generated automatically here.

    ////@begin LoginDialog content initialisation
    ////@end LoginDialog content initialisation
#ifndef ENABLE_SMARTCARD
    m_pCtrlUseSmartCard->Enable(false);
#endif
    m_pCtrlSessionName->Append(m_aSessionNames);
}

/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_SMARTCARD
 */

void LoginDialog::OnCheckboxSmartcardClick( wxCommandEvent& event )
{
    event.Skip();
}


/*!
 * wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_GUESTLOGIN
 */

void LoginDialog::OnCheckboxGuestloginClick( wxCommandEvent& event )
{
    if (m_pCtrlGuestLogin->IsChecked()) {
        m_sTmpUsername = m_pCtrlUsername->GetValue();
        m_pCtrlUsername->SetValue(wxT(""));
        m_pCtrlUsername->Enable(false);
        m_sTmpPassword = m_pCtrlPassword->GetValue();
        m_pCtrlPassword->SetValue(wxT(""));
        m_pCtrlUsername->Enable(false);
    } else {
        m_pCtrlUsername->SetValue(m_sTmpUsername);
        m_pCtrlUsername->Enable(true);
        m_sTmpPassword = m_pCtrlPassword->GetValue();
        m_pCtrlPassword->SetValue(m_sTmpPassword);
        m_pCtrlUsername->Enable(true);
    }
    event.Skip();
}

/*!
 * Should we show tooltips?
 */

bool LoginDialog::ShowToolTips()
{
    return TRUE;
}

/*!
 * Get bitmap resources
 */

wxBitmap LoginDialog::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
    return CreateBitmapFromFile(name);
}

/*!
 * Get icon resources
 */

wxIcon LoginDialog::GetIconResource( const wxString& name )
{
    // Icon retrieval
    return CreateIconFromFile(name);
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_CONFIGURE
 */

void LoginDialog::OnButtonConfigureClick( wxCommandEvent& event )
{
    // Insert custom code here
    SessionProperties d;
    d.SetConfig(m_pCurrentCfg);
    wxString fn = m_pCurrentCfg->sGetFileName();
    d.Create(this);
    switch (d.ShowModal()) {
        case wxID_CANCEL:
            delete m_pCurrentCfg;
            m_pCurrentCfg = new MyXmlConfig(fn);
            break;
        case wxID_CLEAR:
            ::wxLogTrace(MYTRACETAG, wxT("deleting '%s'"), fn.c_str());
            ::wxRemoveFile(fn);
            m_sSessionName.Empty();
            m_pCtrlSessionName->Clear();
            ReadConfigDirectory();
            m_pCtrlSessionName->Append(m_aSessionNames);
            if (!m_sSessionName.IsEmpty())
                m_pCtrlSessionName->SetValue(m_sSessionName);
            break;
        case wxID_OK:
#ifdef ENABLE_SMARTCARD
            m_bUseSmartCard = m_pCurrentCfg->bGetUseSmartCard();
            m_pCtrlUseSmartCard->SetValue(m_bUseSmartCard);
#endif
            if (!m_pCurrentCfg->SaveToFile())
                wxMessageBox(wxString::Format(_("Could not save session to\n%s"),
                            m_pCurrentCfg->sGetFileName().c_str()), _("Error saving - OpenNX"),
                        wxICON_ERROR | wxOK);
            wxConfigBase::Get()->Write(wxT("Config/UserNxDir"), d.GetsUserNxDir());
            wxConfigBase::Get()->Write(wxT("Config/SystemNxDir"), d.GetsSystemNxDir());
            break;
    }
    event.Skip();
}

/*!
 * wxEVT_COMMAND_COMBOBOX_SELECTED event handler for ID_COMBOBOX_SESSION
 */

void LoginDialog::OnComboboxSessionSelected( wxCommandEvent& event )
{
    wxString sessionName = event.GetString();
    if (m_pCurrentCfg)
        delete m_pCurrentCfg;
    for (size_t i = 0; i < m_aConfigFiles.GetCount(); i++) {
        MyXmlConfig cfg(m_aConfigFiles[i]);
        if (cfg.IsValid()) {
            if (cfg.sGetName() == sessionName) {
                m_pCurrentCfg = new MyXmlConfig(m_aConfigFiles[i]);
                m_pCtrlUsername->SetValue(cfg.sGetUsername());
                m_pCtrlPassword->SetValue(cfg.sGetPassword());
#ifdef ENABLE_SMARTCARD
                m_pCtrlUseSmartCard->SetValue(cfg.bGetUseSmartCard());
#endif
            }
        }
    }
    event.Skip();
}

/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void LoginDialog::OnOkClick( wxCommandEvent& event )
{
    if (m_pCurrentCfg) {
        TransferDataFromWindow();
        m_pCurrentCfg->sSetUsername(m_sUsername);
        if (m_pCurrentCfg->bGetRememberPassword())
            m_pCurrentCfg->sSetPassword(m_sPassword);
        m_pCurrentCfg->bSetUseSmartCard(m_bUseSmartCard);
        if (m_bUseSmartCard)
            m_pCurrentCfg->bSetEnableSSL(true);

        if (!m_pCurrentCfg->SaveToFile())
            wxMessageBox(wxString::Format(_("Could not save session to\n%s"),
                        m_pCurrentCfg->sGetFileName().c_str()), _("Error saving - OpenNX"),
                    wxICON_ERROR | wxOK);
        else {
            MySession s;
            s.Create(m_pCurrentCfg->sGetFileName(), m_sPassword);
        }
    }
    event.Skip();
}
