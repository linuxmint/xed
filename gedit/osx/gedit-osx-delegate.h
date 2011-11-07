#ifndef GEDIT_OSX_DELEGATE_H_
#define GEDIT_OSX_DELEGATE_H_

#import <Foundation/NSAppleEventManager.h>

@interface GeditOSXDelegate : NSObject
{
}

-(id) init;
-(void) openFiles:(NSAppleEventDescriptor*)event
        withReply:(NSAppleEventDescriptor*)reply;

@end

#endif /* GEDIT_OSX_DELEGATE_H_ */
