/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#import "MacUIPanel.h"

@protocol MacPINPanelDelegate;

@interface MacPINPanel : NSObject <MacUIPanel>
{
	@private
	id <MacPINPanelDelegate> m_delegate;
	id m_userInfo;
	IBOutlet NSWindow *m_window;
	IBOutlet NSButton *m_cancelButton;
	IBOutlet NSButton *m_detailsButton;
	IBOutlet NSBox *m_detailsBox;
	IBOutlet NSTextField *m_detailsLabel;
	IBOutlet NSTextField *m_errorLabel;
	IBOutlet NSTextField *m_hashLabel;
	IBOutlet NSTextField *m_hashTextField;
	IBOutlet NSTextField *m_messageLabel;
	IBOutlet NSTextField *m_nameLabel;
	IBOutlet NSTextField *m_nameTextField;
	IBOutlet NSButton *m_okButton;
	IBOutlet NSTextField *m_pinLabel;
	IBOutlet NSSecureTextField *m_pinTextField;
	IBOutlet NSTextField *m_urlLabel;
	IBOutlet NSTextField *m_urlTextField;
}

- (id <MacPINPanelDelegate>)delegate;
- (void)setDelegate:(id <MacPINPanelDelegate>)delegate;
- (id)userInfo;
- (void)setUserInfo:(id)userInfo;

- (BOOL)showsDetails;
- (void)setShowsDetails:(BOOL)flag animate:(BOOL)animate;

- (BOOL)allowsSecureEntry;
- (void)setAllowsSecureEntry:(BOOL)allowsSecureEntry;
- (NSString *)error;
- (void)setError:(NSString *)error fatal:(BOOL)fatal;
- (NSString *)hash;
- (void)setHash:(NSString *)hash;
- (NSString *)name;
- (void)setName:(NSString *)name;
- (NSString *)PIN;
- (void)setPIN:(NSString *)PIN;
- (NSString *)URL;
- (void)setURL:(NSString *)url;

/**
 * @name Actions
 */

- (IBAction)cancel:(id)sender;
- (IBAction)ok:(id)sender;
- (IBAction)showHelp:(id)sender;
- (IBAction)showOptions:(id)sender;
- (IBAction)toggleDetails:(id)sender;

@end
