#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@protocol MacUIPanel <NSObject>

- (NSWindow *)window;
- (void)beginSheetForWindow:(NSWindow *)window modalDelegate:(id)delegate didEndSelector:(SEL)selector contextInfo:(void *)info;

@end
