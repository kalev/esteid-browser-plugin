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

@interface MacSettingsPanel : NSObject <MacUIPanel>
{
	@private
	IBOutlet NSTableView *m_tableView;
	IBOutlet NSWindow *m_window;
	IBOutlet NSButton *m_cancelButton;
	IBOutlet NSButton *m_okButton;
	IBOutlet NSTextField *m_messageLabel;
	NSMutableArray *m_websites;
	NSString *m_website;
	BOOL m_shouldSaveOnClose;
}

- (BOOL)shouldSaveOnClose;
- (NSString *)website;
- (void)setWebsite:(NSString *)website;
- (NSArray *)websites;
- (void)setWebsites:(NSArray *)websites;

- (IBAction)cancel:(id)sender;
- (IBAction)ok:(id)sender;
- (IBAction)showHelp:(id)sender;

@end
