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

#import "MacSettingsPanel.h"

static NSString *MacSettingsPanelWebsiteColumn = @"website";

@implementation MacSettingsPanel

- (BOOL)shouldSaveOnClose
{
	return self->m_shouldSaveOnClose;
}

- (NSString *)website
{
	return self->m_website;
}

- (void)setWebsite:(NSString *)website
{
	if(self->m_website != website) {
		[self->m_website release];
		self->m_website = [website retain];
		
		if([self->m_website length] > 0) {
			NSBundle *bundle = [NSBundle bundleForClass:[self class]];
			
			[self->m_messageLabel setStringValue:[NSString stringWithFormat:[bundle localizedStringForKey:@"SettingsPanel.Message.Format" value:nil table:nil], self->m_website]];
		} else {
			[self->m_messageLabel setStringValue:@""];
		}
	}
}

- (NSArray *)websites
{
	return [NSArray arrayWithArray:self->m_websites];
}

- (void)setWebsites:(NSArray *)websites
{
	[self->m_websites removeAllObjects];
	
	if([websites count] > 0) {
		[self->m_websites addObjectsFromArray:websites];
	}
}

- (IBAction)cancel:(id)sender
{
	self->m_shouldSaveOnClose = NO;
	
	[[self retain] autorelease];
	[[self window] orderOut:sender];
	[[NSApplication sharedApplication] endSheet:[self window] returnCode:NSRunAbortedResponse];
}

- (IBAction)ok:(id)sender
{
	self->m_shouldSaveOnClose = YES;
	
	// Post notification
	[[NSNotificationCenter defaultCenter] postNotificationName:@"WhitelistOKPressed" object:self];

	[[self retain] autorelease];
	[[self window] orderOut:sender];
	[[NSApplication sharedApplication] endSheet:[self window] returnCode:NSRunStoppedResponse];
}

- (IBAction)showHelp:(id)sender
{
}

#pragma mark MacUIPanel

- (NSWindow *)window
{
	return self->m_window;
}

- (void)beginSheetForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)selector contextInfo:(void *)info
{
	[[NSApplication sharedApplication] beginSheet:[self window] modalForWindow:window modalDelegate:delegate didEndSelector:selector contextInfo:info];
}

#pragma mark NSTableDataSource

- (int)numberOfRowsInTableView:(NSTableView *)tableView
{
	return [self->m_websites count] + 1;
}

- (void)tableView:(NSTableView *)tableView willDisplayCell:(id)cell forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	if([[tableColumn identifier] isEqualToString:MacSettingsPanelWebsiteColumn]) {
		NSBundle *bundle = [NSBundle bundleForClass:[self class]];
		
		[cell setPlaceholderString:[bundle localizedStringForKey:@"SettingsPanel.Placeholder.Website" value:nil table:nil]];
	}
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	if([[tableColumn identifier] isEqualToString:MacSettingsPanelWebsiteColumn]) {
		if(rowIndex < [self->m_websites count]) {
			return [self->m_websites objectAtIndex:rowIndex];
		}
	}
	
	return @"";
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)tableColumn row:(int)rowIndex
{
	if([[tableColumn identifier] isEqualToString:MacSettingsPanelWebsiteColumn]) {
		NSString *value = [anObject stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
		
#if 0
		// Lets make it more user-proof
		if([value length] > 3 && [value rangeOfString:@"://"].location == NSNotFound) {
			value = [@"https://" stringByAppendingString:value];
		}
		
		// Don't forget the slash
		if([value length] > 10 && [[value componentsSeparatedByString:@"/"] count] <= 2 && ![value hasSuffix:@"/"]) {
			value = [value stringByAppendingString:@"/"];
		}
#endif
		
		if(rowIndex < [self->m_websites count]) {
			if([value length] == 0) {
				[self->m_websites removeObjectAtIndex:rowIndex];
				[self->m_tableView reloadData];
			} else if(![self->m_websites containsObject:value]) {
				[self->m_websites replaceObjectAtIndex:rowIndex withObject:value];
				[self->m_tableView reloadData];
			}
		} else {
			if([value length] > 0 && ![self->m_websites containsObject:value]) {
				[self->m_websites addObject:value];
				[self->m_tableView reloadData];
			}
		}
	}
}

#pragma mark NSTableView+Delegate

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
}

- (void)tableViewOnDelete:(NSTableView *)tableView row:(int)rowIndex
{
	if(rowIndex < [self->m_websites count]) {
		[self->m_websites removeObjectAtIndex:rowIndex];
		[self->m_tableView reloadData];
	}
}

#pragma mark NSWindowController

- (void)awakeFromNib
{
	NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	
	[self->m_cancelButton setStringValue:[bundle localizedStringForKey:@"SettingsPanel.Action.Cancel" value:nil table:nil]];
	[self->m_okButton setStringValue:[bundle localizedStringForKey:@"SettingsPanel.Action.OK" value:nil table:nil]];
	[self->m_window setTitle:[bundle localizedStringForKey:@"SettingsPanel.Title" value:nil table:nil]];
}

#pragma mark NSObject

- (id)init
{
	self = [super init];
	
	if(self) {
		self->m_websites = [[NSMutableArray alloc] init];
		
		[NSBundle loadNibNamed:@"SettingsPanel" owner:self];
	}
	
	return self;
}

- (void)dealloc
{
	[self->m_websites release];
	[self->m_website release];
	
	[super dealloc];
}

@end
