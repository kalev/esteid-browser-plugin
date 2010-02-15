#import "MacUIPrivate.h"

@implementation MacUIPrivate

#pragma mark MacPINPanelDelegate

- (BOOL)pinPanelShouldEnd:(MacPINPanel *)pinPanel
{
	/*NSError *error = nil;
	 
	 //[pinPanel setUserInfo:[[[self->m_readerManager selectedReader] sign:[pinPanel hash] pin:[pinPanel PIN] error:&error] uppercaseString]]; 
	 
	 if([[error domain] isEqualToString:EstEIDReaderErrorDomain]) {
	 NSBundle *bundle = [NSBundle bundleForClass:[self class]];
	 
	 NSBeep();
	 
	 switch([error code]) {
	 case EstEIDReaderErrorLockedPIN:
	 [pinPanel setError:[bundle localizedStringForKey:@"PINPanel.Error.PIN2.Locked" value:nil table:nil] fatal:YES];
	 break;
	 case EstEIDReaderErrorInvalidPIN:
	 [pinPanel setError:[bundle localizedStringForKey:@"PINPanel.Error.PIN2.Invalid" value:nil table:nil] fatal:NO];
	 break;
	 }
	 }*/
	
	return ([pinPanel userInfo]) ? YES : NO;
}

@end
