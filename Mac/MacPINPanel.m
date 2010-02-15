#import "MacPINPanel.h"

static NSString *MacPINPanelShowsDetailsKey = @"MacPINPanelShowsDetails";

@implementation MacPINPanel

- (NSWindow *)window
{
	return self->m_window;
}

- (id <MacPINPanelDelegate>)delegate
{
	return self->m_delegate;
}

- (void)setDelegate:(id <MacPINPanelDelegate>)delegate
{
	self->m_delegate = delegate;
}

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

- (void)beginSheetForWindow:(NSWindow *)window
{
	[self beginSheetForWindow:window modalDelegate:nil didEndSelector:nil contextInfo:NULL];
}

- (void)beginSheetForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)selector contextInfo:(void *)info
{
	[[NSApplication sharedApplication] beginSheet:[self window] modalForWindow:window modalDelegate:delegate didEndSelector:selector contextInfo:info];
	
	if([self allowsSecureEntry]) {
		[self->m_nameTextField selectText:nil];
	}
}

- (void)runModal
{
	NSApplication *application = [NSApplication sharedApplication];
	
	while([self->m_window isVisible]) {
		NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
		NSDate *date = [[NSDate alloc] initWithTimeIntervalSinceNow:0.1F];
		NSEvent *event = [application nextEventMatchingMask:NSAnyEventMask untilDate:date inMode:NSDefaultRunLoopMode dequeue:YES];
		
		if(event) {
			[application sendEvent:event];
		}
		
		[date release];
		[pool release];
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
	
	if(![self->m_delegate respondsToSelector:@selector(pinPanelShouldEnd:)] || [self->m_delegate pinPanelShouldEnd:self]) {
		[[self retain] autorelease];
		[[self window] orderOut:sender];
		[[NSApplication sharedApplication] endSheet:[self window] returnCode:NSRunStoppedResponse];
	}
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
