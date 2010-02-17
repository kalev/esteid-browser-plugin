#import "MacUIPanel.h"

@interface MacSettingsPanel : NSObject <MacUIPanel>
{
	@private
	IBOutlet NSTableView *m_tableView;
	IBOutlet NSWindow *m_window;
	IBOutlet NSButton *m_cancelButton;
	IBOutlet NSButton *m_okButton;
	IBOutlet NSTextField *m_messageLabel;
	NSMutableArray *m_websites;
	NSString *m_website;
	BOOL m_shouldSaveOnClose;
}

- (BOOL)shouldSaveOnClose;
- (NSString *)website;
- (void)setWebsite:(NSString *)website;
- (NSArray *)websites;
- (void)setWebsites:(NSArray *)websites;

- (IBAction)cancel:(id)sender;
- (IBAction)ok:(id)sender;
- (IBAction)showHelp:(id)sender;

@end
