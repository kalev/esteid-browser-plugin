#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@class MacPINPanel;

@protocol MacPINPanelDelegate <NSObject>

- (BOOL)pinPanelShouldEnd:(MacPINPanel *)pinPanel;

@end
