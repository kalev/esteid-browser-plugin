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

#import "MacUI.h"
#import "MacPINPanel.h"
#import "MacSettingsPanel.h"
#import "MacUIPrivate.h"
#import "Mac/PluginWindowMacQuickDraw.h"

#import "debug.h"

static inline NSString *CPlusStringToNSString(std::string str)
{
    return [(NSString *)CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)str.c_str(), str.length(), kCFStringEncodingISOLatin9, NO) autorelease];
}

#pragma mark -

void MacUI::SetWindow(void *window)
{
#ifndef __LP64__
    NSLog(@"Khuul !");
#endif
}

MacUI::MacUI(boost::shared_ptr<UICallbacks> cb) : PluginUI(cb)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    m_internal = [[MacUIPrivate alloc] init];

    [(MacUIPrivate *)m_internal registerCallbacks:m_callbacks];
    
    [pool release];
    
    ESTEID_DEBUG("MacUI intialized");
}

MacUI::~MacUI()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    [(id)m_internal release];
    [pool release];
    
    ESTEID_DEBUG_SCOPE();
}

void MacUI::pinDialog(const std::string& subject,
                      const std::string& docUrl,
                      const std::string& docHash,
                      bool retry, int tries)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    if (![(MacUIPrivate *)m_internal isLocked]) {
        MacPINPanel *panel = [[MacPINPanel alloc] init];
        NSString *result;

        [panel setAllowsSecureEntry:NO];
        [panel setHash:CPlusStringToNSString(docHash)];
        [panel setURL:CPlusStringToNSString(docUrl)];
        [panel setName:CPlusStringToNSString(subject)];

        if (retry) {
            NSBundle *bundle = [NSBundle bundleForClass:[(MacUIPrivate *)m_internal class]];
            NSString *error = nil;

            error = [bundle localizedStringForKey:@"PINPanel.Error.PIN2.Invalid" value:nil table:nil];
            NSBeep();
            [panel setError:error fatal:NO];
        }

        /* MacUIPrivate will handle releasing panel */
        [(MacUIPrivate *)m_internal runAsyncPinPanel:panel];
    }

    [pool release];
}

#ifdef SUPPORT_OLD_APIS
std::string MacUI::PromptForPin(const std::string& subject,
                                const std::string& docUrl, const std::string& docHash,
                                bool retry, int tries)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    std::string pin = "";
    
    if(![(MacUIPrivate *)m_internal isLocked]) {
        MacPINPanel *panel = [[MacPINPanel alloc] init];
        NSString *result;
        
        [panel setAllowsSecureEntry:NO];
        [panel setHash:CPlusStringToNSString(docHash)];
        [panel setURL:CPlusStringToNSString(docUrl)];
        [panel setName:CPlusStringToNSString(subject)];
        
        [(MacUIPrivate *)m_internal runModal:panel];
        result = [panel PIN];
        
        if([result length] > 0) {
            pin = std::string([result UTF8String]);
        }
        
        [panel release];
    }
    
    [pool release];
    
    return pin;
}
#endif

void MacUI::closePinDialog()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    [(MacUIPrivate *)m_internal abortModal];
    
    [pool release];
}

void MacUI::pinBlockedMessage(int pin)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSBundle *bundle = [NSBundle bundleForClass:[(MacUIPrivate *)m_internal class]];
    NSAlert *alert = [NSAlert alertWithMessageText:[bundle localizedStringForKey:@"Alert.PINBlocked.Message" value:nil table:nil]
                                     defaultButton:[bundle localizedStringForKey:@"Alert.Action.OK" value:nil table:nil]
                                   alternateButton:nil
                                       otherButton:nil
                         informativeTextWithFormat:@"", nil];
    
    [alert setIcon:[[[NSImage alloc] initWithContentsOfFile:[bundle pathForResource:@"Icon" ofType:@"png"]] autorelease]];
    
    [(MacUIPrivate *)m_internal runModal:(id)alert];
    
    [pool release];
}

void MacUI::settingsDialog(PluginSettings& settings, const std::string& pageUrl)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    if(![(MacUIPrivate *)m_internal isLocked]) {
        [(MacUIPrivate *)m_internal setConf:&settings];

        MacSettingsPanel *panel = [[MacSettingsPanel alloc] init];
        NSMutableArray *websites = [NSMutableArray array];
        
        std::vector<std::string> whitelist = settings.whitelist();
        for (std::vector<std::string>::const_iterator it = whitelist.begin(); it != whitelist.end(); it++) {
            [websites addObject:CPlusStringToNSString(*it)];
        }
        
        [panel setWebsite:CPlusStringToNSString(pageUrl)];
        [panel setWebsites:websites];
        [(MacUIPrivate *)m_internal runAsyncWhitelist:panel];
    }
    
    [pool release];
}
