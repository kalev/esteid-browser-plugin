#import "MacPINPanelDelegate.h"

@protocol MacUIPanel;

@interface MacUIPrivate : NSObject <MacPINPanelDelegate>
{
	@private
	BOOL m_abort;
	BOOL m_locked;
	NSWindow *m_window;
}

- (NSWindow *)window;

- (BOOL)isLocked;
- (void)setLocked:(BOOL)locked;

- (void)abortModal;
- (void)runModal:(id <MacUIPanel>)panel;

@end
