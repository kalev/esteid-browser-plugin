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
#import "MacUIPrivate.h"
#import "MacPINPanel.h"
#import "EstEIDService.h"

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

- (void)abortModal
{
	if(self->m_locked) {
		self->m_abort = YES;
	}
}

- (void)runModal:(id <MacUIPanel>)panel
{
	if(!self->m_locked) {
		self->m_locked = YES;
		
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

#pragma mark MacPINPanelDelegate

- (BOOL)pinPanelShouldEnd:(MacPINPanel *)pinPanel
{
	NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	NSString *error = nil;
	BOOL fatal = NO;
	
	try {
		EstEIDService *service = EstEIDService::getInstance();
		
		[pinPanel setUserInfo:CPlusStringToNSString(service->signSHA1([[pinPanel hash] UTF8String], EstEidCard::SIGN, [[pinPanel PIN] UTF8String]))];
	}
	catch(AuthError err) {
		NSLog(@"%@: Couldn't sign hash %@ because of '%s'.", NSStringFromClass([self class]), [pinPanel hash], err.what());
		
		if(err.m_blocked) {
			error = [bundle localizedStringForKey:@"PINPanel.Error.PIN2.Locked" value:nil table:nil];
			fatal = YES;
		} else {
			error = [bundle localizedStringForKey:@"PINPanel.Error.PIN2.Invalid" value:nil table:nil];
			fatal = NO;
		}
	}
	catch(std::runtime_error err) {
		NSLog(@"%@: Couldn't sign hash %@ because of '%s'", NSStringFromClass([self class]), [pinPanel hash], err.what());
		fatal = YES;
	}
	
	if(error || fatal) {
		NSBeep();
		
		if(error) {
			[pinPanel setError:error fatal:fatal];
		}
		
		return NO;
	}
	
	return YES;
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
