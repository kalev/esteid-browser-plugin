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

#import "MacPINPanel.h"

static NSString *MacPINPanelShowsDetailsKey = @"MacPINPanelShowsDetails";

@implementation MacPINPanel

- (BOOL)allowsSecureEntry
{
	return ([[[self->m_pinTextField cell] placeholderString] length] > 0) ? YES : NO;
}

- (void)setAllowsSecureEntry:(BOOL)allowsSecureEntry
{
	if(allowsSecureEntry) {
		NSBundle *bundle = [NSBundle bundleForClass:[self class]];
		
		[[self->m_pinTextField cell] setPlaceholderString:[bundle localizedStringForKey:@"PINPanel.Label.SecureEntry" value:nil table:nil]];
	} else {
		[[self->m_pinTextField cell] setPlaceholderString:@""];
	}
}

- (id)userInfo
{
	return self->m_userInfo;
}

- (void)setUserInfo:(id)userInfo
{
	if(self->m_userInfo != userInfo) {
		[self->m_userInfo release];
		self->m_userInfo = [userInfo retain];
	}
}

- (BOOL)showsDetails
{
	return ([self->m_detailsBox isHidden] == NO) ? YES : NO;
}

- (void)setShowsDetails:(BOOL)flag animate:(BOOL)animate
{
	if([self showsDetails] != flag) {
		float height = [self->m_detailsBox frame].size.height;
		NSRect frame = [self->m_window frame];
		
		if([self->m_detailsBox isHidden]) {
			frame.origin.y -= height;
			frame.size.height += height;
			
			[self->m_window setFrame:frame display:animate animate:animate];
			[self->m_detailsBox setHidden:NO];
			[self->m_detailsButton setState:NSOnState];
		} else {
			frame.origin.y += height;
			frame.size.height -= height;
			
			[self->m_detailsBox setHidden:YES];
			[self->m_detailsButton setState:NSOffState];
			[self->m_window setFrame:frame display:animate animate:animate];
		}
		
		[[NSUserDefaults standardUserDefaults] setBool:flag forKey:MacPINPanelShowsDetailsKey];
	}
}

- (NSString *)error
{
	return [self->m_errorLabel stringValue];
}

- (void)setError:(NSString *)error fatal:(BOOL)fatal
{
	if([error length] > 0) {
		[self->m_errorLabel setStringValue:error];
		[self->m_okButton setEnabled:!fatal];
	} else {
		[self->m_errorLabel setStringValue:@""];
		[self->m_okButton setEnabled:YES];
	}
}

- (NSString *)hash
{
	return [self->m_hashTextField stringValue];
}

- (void)setHash:(NSString *)hash
{
	[self->m_hashTextField setStringValue:(hash) ? hash : @""];
}

- (NSString *)name
{
	return [self->m_nameTextField stringValue];
}

- (void)setName:(NSString *)name
{
	[self->m_nameTextField setStringValue:(name) ? name : @""];
}

- (NSString *)PIN
{
	return [self->m_pinTextField stringValue];
}

- (void)setPIN:(NSString *)PIN
{
	[self->m_pinTextField setStringValue:(PIN) ? PIN : @""];
}

- (NSString *)URL
{
	return [self->m_urlTextField stringValue];
}

- (void)setURL:(NSString *)URL
{
	[self->m_urlTextField setStringValue:(URL) ? URL : @""];
}

- (IBAction)cancel:(id)sender
{
	[[self retain] autorelease];
	[[self window] orderOut:sender];

	// Post notification
	[[NSNotificationCenter defaultCenter] postNotificationName:@"PinPanelCancel" object:self];

	[[NSApplication sharedApplication] endSheet:[self window] returnCode:NSRunAbortedResponse];
}

- (IBAction)ok:(id)sender
{
	//NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	BOOL allowsSecureEntry = [self allowsSecureEntry];
	NSString *pin = [self PIN];
	
	if([pin length] == 0) {
		if(!allowsSecureEntry) {
			//[bundle localizedStringForKey:@"PINPanel.Error.PIN.None" value:nil table:nil];
			NSBeep();
			return;
		}
	} else if([pin length] < 5 || [pin length] > 12) {
		//[bundle localizedStringForKey:@"PINPanel.Error.PIN.Length" value:nil table:nil];
		NSBeep();
		return;
	}
	
	// Post notification with entered PIN
	NSDictionary *dict = [NSDictionary dictionaryWithObject:pin forKey:@"PIN"];
	[[NSNotificationCenter defaultCenter] postNotificationName:@"PinPanelOK" object:self userInfo:dict];

	[[self retain] autorelease];
	[[self window] orderOut:sender];
	[[NSApplication sharedApplication] endSheet:[self window] returnCode:NSRunStoppedResponse];
}

- (IBAction)showHelp:(id)sender
{
	NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	id site = [[bundle localizedInfoDictionary] objectForKey:@"EstEIDHelpWebsite"];
	
	if(!site) {
		site = [[bundle infoDictionary] objectForKey:@"EstEIDHelpWebsite"];
	}
	
	if(!([site isKindOfClass:[NSString class]] && [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:(NSString *)site]])) {
		NSLog(@"%@: Couldn't open help site (%@)!", NSStringFromClass([self class]), site);
		NSBeep();
	}
}

- (IBAction)showOptions:(id)sender
{
	// Just download the file at the moment
	NSURL *url = [NSURL URLWithString:[self URL]];
	
	if(url) {
		[[NSWorkspace sharedWorkspace] openURL:url];
	} else {
		NSBeep();
	}
}

- (IBAction)toggleDetails:(id)sender
{
	[self setShowsDetails:![self showsDetails] animate:YES];
}

#pragma mark MacUIPanel

- (NSWindow *)window
{
	return self->m_window;
}

- (void)beginSheetForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)selector contextInfo:(void *)info
{
	[[NSApplication sharedApplication] beginSheet:[self window] modalForWindow:window modalDelegate:delegate didEndSelector:selector contextInfo:info];
	
	if([self allowsSecureEntry]) {
		[self->m_nameTextField selectText:nil];
	}
}

#pragma mark NSWindowController

- (void)awakeFromNib
{
	NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	
	[self->m_urlLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Label.URL" value:nil table:nil]];
	[self->m_hashLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Label.Hash" value:nil table:nil]];
	[self->m_nameLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Label.Name" value:nil table:nil]];
	[self->m_pinLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Label.PIN2" value:nil table:nil]];
	[self->m_messageLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Message.PIN2" value:nil table:nil]];
	[self->m_detailsLabel setStringValue:[bundle localizedStringForKey:@"PINPanel.Label.Details" value:nil table:nil]];
	[self->m_cancelButton setStringValue:[bundle localizedStringForKey:@"PINPanel.Action.Cancel" value:nil table:nil]];
	[self->m_okButton setStringValue:[bundle localizedStringForKey:@"PINPanel.Action.OK" value:nil table:nil]];
	[self->m_window setTitle:[bundle localizedStringForKey:@"PINPanel.Title" value:nil table:nil]];
	
	[self setShowsDetails:[[NSUserDefaults standardUserDefaults] boolForKey:MacPINPanelShowsDetailsKey] animate:NO];
}

#pragma mark NSObject

- (id)init
{
	self = [super init];
	
	if(self) {
		[NSBundle loadNibNamed:@"PINPanel" owner:self];
	}
	
	return self;
}

- (void)dealloc
{
	[self->m_userInfo autorelease];
	[self->m_window release];
	
	[super dealloc];
}

@end
