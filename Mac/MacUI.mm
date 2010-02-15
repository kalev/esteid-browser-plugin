#import "MacUI.h"
#import "MacUIPrivate.h"
#import "Mac/PluginWindowMac.h"

#define ESTEID_DEBUG printf

static inline NSString *CPlusStringToNSString(std::string str)
{
	return [(NSString *)CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *)str.c_str(), str.length(), kCFStringEncodingISOLatin9, NO) autorelease];
}

#pragma mark -

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
	MacPINPanel *panel = [[MacPINPanel alloc] init];
	NSString *result;
	std::string pin;
	
	[panel setAllowsSecureEntry:(pinPadTimeout > 0) ? YES : NO];
	[panel setDelegate:(id)m_internal];
	[panel setHash:CPlusStringToNSString(docHash)];
	[panel setURL:CPlusStringToNSString(docUrl)];
	[panel setName:CPlusStringToNSString(subject)];
	
#if 0
	// TODO: Maybe we should display a sheet under 10.4/10.5?
	[panel beginSheetForWindow:BROWSERWINDOW];
#else
	// EPIC: Out-of-process solution (64b 10.6+)
	[[panel window] setLevel:NSPopUpMenuWindowLevel];
	[[panel window] orderFront:nil];
#endif
	
	[panel runModal];
	result = [panel userInfo];
	
	if([result length] > 0) {
		pin = std::string([result UTF8String]);
	} else {
		pin = "";
	}
	
	[pool release];
	
    return pin;
}

void MacUI::ClosePinPrompt()
{
}

void MacUI::ShowPinBlockedMessage(int pin)
{
}

void MacUI::ShowSettings(PluginSettings &conf, std::string pageUrl)
{
}
