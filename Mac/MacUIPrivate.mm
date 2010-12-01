/*
 * esteid-browser-plugin - a browser plugin for Estonian EID card
 *
 * Copyright (C) 2010  Estonian Informatics Centre
 * Copyright (C) 2010  Smartlink OÜ
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
#import "MacUIPrivate.h"
#import "MacPINPanel.h"
#import "MacSettingsPanel.h"
#import "MacUI.h"

static inline NSString *CPlusStringToNSString(std::string str)
{
	return [(NSString *)CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)str.c_str(), str.length(), kCFStringEncodingISOLatin9, NO) autorelease];
}

#pragma mark -

@implementation MacUIPrivate

- (NSWindow *)window
{
	return self->m_window;
}

- (BOOL)isLocked
{
	return self->m_locked;
}

- (void)setLocked:(BOOL)locked
{
	self->m_locked = locked;
}

- (void)registerCallbacks:(boost::shared_ptr<MacUI::UICallbacks>)cb
{
	self->m_callbacks = cb;
}

- (void)setConf:(PluginSettings *)settings
{
	self->m_settings = settings;
}

- (void)abortModal
{
	if(self->m_locked) {
		self->m_abort = YES;
	}
}

- (void)runAsync:(id <MacUIPanel>)panel
{
	if (!self->m_locked) {
		self->m_locked = YES;
		self->m_async = YES;

		@try {
			NSApplication *application = [NSApplication sharedApplication];
			NSWindow *window = [panel window];

			if (self->m_window) {
				[panel beginSheetForWindow:self->m_window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
			} else {
				// EPIC: Out-of-process solution (64b 10.6+)
				if ([panel respondsToSelector:@selector(runModal)]) {
					[(NSAlert *)panel runModal];
				} else {
					[window setLevel:NSPopUpMenuWindowLevel];
					[window orderFront:nil];
				}
			}
		} @catch(NSException *e) {
			NSLog(@"%@: %@", NSStringFromClass([self class]), e);
		}
	}
}

- (void)runAsyncPinPanel:(id <MacUIPanel>)panel
{
	if (self->m_locked)
		return;

	// Register observer to be called when OK is pressed
	[[NSNotificationCenter defaultCenter] addObserver:self
	                       selector:@selector(pinPanelOKPressed:)
	                       name:@"PinPanelOK"
	                       object:nil];

	// Register observer to be called when Cancel is pressed
	[[NSNotificationCenter defaultCenter] addObserver:self
	                       selector:@selector(pinPanelCancelPressed:)
	                       name:@"PinPanelCancel"
	                       object:nil];

        [self runAsync:panel];
}

- (void)runAsyncWhitelist:(id <MacUIPanel>)panel
{
	if (self->m_locked)
		return;

	// Register observer to be called when whitelist is closed
	[[NSNotificationCenter defaultCenter] addObserver:self
	                       selector:@selector(whitelistOKPressed:)
	                       name:@"WhitelistOKPressed"
	                       object:nil];

	// Register observer to be called when whitelist is closed
	[[NSNotificationCenter defaultCenter] addObserver:self
	                       selector:@selector(whitelistCancelPressed:)
	                       name:@"WhitelistCancelPressed"
	                       object:nil];

        [self runAsync:panel];
}

- (void)runModal:(id <MacUIPanel>)panel
{
	if(!self->m_locked) {
		self->m_locked = YES;
		self->m_async = NO;
		
		@try {
			NSApplication *application = [NSApplication sharedApplication];
			NSWindow *window = [panel window];
			
			if(self->m_window) {
				[panel beginSheetForWindow:self->m_window modalDelegate:nil didEndSelector:NULL contextInfo:NULL];
			} else {
				// EPIC: Out-of-process solution (64b 10.6+)
				if([panel respondsToSelector:@selector(runModal)]) {
					[(NSAlert *)panel runModal];
				} else {
					[window setLevel:NSPopUpMenuWindowLevel];
					[window orderFront:nil];
				}
			}
			
			while([window isVisible]) {
				NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
				NSDate *date = [[NSDate alloc] initWithTimeIntervalSinceNow:0.1F];
				NSEvent *event = [application nextEventMatchingMask:NSAnyEventMask untilDate:date inMode:NSDefaultRunLoopMode dequeue:YES];
				
				if(event) {
					[application sendEvent:event];
				}
				
				if(self->m_abort) {
					[window orderOut:nil];
				}
				
				[date release];
				[pool release];
			}
		}
		@catch(NSException *e) {
			NSLog(@"%@: %@", NSStringFromClass([self class]), e);
		}
		
		self->m_locked = NO;
		self->m_abort = NO;
	}
}

- (void)pinPanelOKPressed:(NSNotification *)notification
{
	if (self->m_async) {
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		self->m_locked = NO;
		self->m_abort = NO;
		self->m_callbacks->onPinEntered([[[notification userInfo] valueForKey:@"PIN"] UTF8String]);
	}
}

- (void)pinPanelCancelPressed:(NSNotification *)notification
{
	if (self->m_async) {
		[[NSNotificationCenter defaultCenter] removeObserver:self];
		self->m_locked = NO;
		self->m_abort = NO;
		self->m_callbacks->onPinCancelled();
	}
}

- (void)whitelistOKPressed:(NSNotification *)notification
{
	MacSettingsPanel *panel = [notification object];
	NSEnumerator *enumerator = [[panel websites] objectEnumerator];
	NSString *website;

	m_settings->whitelist.clear();
	while ((website = [enumerator nextObject]) != nil) {
		m_settings->whitelist.push_back([website UTF8String]);
	}

	try {
		m_settings->save();
	} catch(const std::exception& err) {
		NSLog(@"%@: Couldn't save configuration!", NSStringFromClass([panel class]));
	}

	[[NSNotificationCenter defaultCenter] removeObserver:self];
	self->m_locked = NO;
	self->m_abort = NO;
}

- (void)whitelistCancelPressed:(NSNotification *)notification
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	self->m_locked = NO;
	self->m_abort = NO;
}

#pragma mark NSObject

- (id)init
{
	self = [super init];
	
	if(self) {
		self->m_locked = NO;
		self->m_window = nil;
	}

	return self;
}

@end
