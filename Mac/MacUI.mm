#import "MacUI.h"
#import "MacPINPanel.h"
#import "MacSettingsPanel.h"
#import "MacUIPrivate.h"
#import "Mac/PluginWindowMac.h"

#define ESTEID_DEBUG printf

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

MacUI::MacUI()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	m_internal = [[MacUIPrivate alloc] init];
	
	[pool release];
	
	ESTEID_DEBUG("MacUI intialized\n");
}

MacUI::~MacUI()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[(id)m_internal release];
	[pool release];
	
	ESTEID_DEBUG("~MacUI()\n");
}

std::string MacUI::PromptForSignPIN(std::string subject,
										std::string docUrl, std::string docHash,
										std::string pageUrl, int pinPadTimeout, bool retry, int tries)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	std::string pin = "";
	
	if(![(MacUIPrivate *)m_internal isLocked]) {
		MacPINPanel *panel = [[MacPINPanel alloc] init];
		NSString *result;
		
		[panel setAllowsSecureEntry:(pinPadTimeout > 0) ? YES : NO];
		[panel setDelegate:(id)m_internal];
		[panel setHash:CPlusStringToNSString(docHash)];
		[panel setURL:CPlusStringToNSString(docUrl)];
		[panel setName:CPlusStringToNSString(subject)];
		
		[(MacUIPrivate *)m_internal runModal:panel];
		result = [panel userInfo];
		
		if([result length] > 0) {
			pin = std::string([result UTF8String]);
		}
		
		[panel release];
	}
	
	[pool release];
	
	return pin;
}

void MacUI::ClosePinPrompt()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	[(MacUIPrivate *)m_internal abortModal];
	
	[pool release];
}

void MacUI::ShowPinBlockedMessage(int pin)
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

void MacUI::ShowSettings(PluginSettings &conf, std::string pageUrl)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	if(![(MacUIPrivate *)m_internal isLocked]) {
		MacSettingsPanel *panel = [[MacSettingsPanel alloc] init];
		NSMutableArray *websites = [NSMutableArray array];
		
		for(std::vector<std::string>::const_iterator it = conf.whitelist.begin(); it != conf.whitelist.end(); it++) {
			[websites addObject:CPlusStringToNSString(*it)];
		}
		
		[panel setWebsite:CPlusStringToNSString(pageUrl)];
		[panel setWebsites:websites];
		[(MacUIPrivate *)m_internal runModal:panel];
		
		if(![websites isEqualToArray:[panel websites]]) {
			NSEnumerator *enumerator = [[panel websites] objectEnumerator];
			NSString *website;
			
			conf.whitelist.clear();
			
			while((website = [enumerator nextObject]) != nil) {
				conf.whitelist.push_back([website UTF8String]);
			}
			
			if([panel shouldSaveOnClose]) {
				try {
					conf.Save();
				}
				catch(std::runtime_error err) {
					NSLog(@"%@: Couldn't save configuration!", NSStringFromClass([panel class]));
				}
			}
		}
		
		[panel release];
	}
	
	[pool release];
}
