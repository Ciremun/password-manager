#ifndef PM_EVENT_H_
#define PM_EVENT_H_

#include "pm_util.h"

void EXPORT("HandleKey") HandleKey(int keycode, int bDown);
void EXPORT("HandleButton") HandleButton(int x, int y, int button, int bDown);
void EXPORT("HandleMotion") HandleMotion(int x, int y, int mask);
void HandleDestroy();
void HandleSuspend();
void HandleResume();

#endif // PM_EVENT_H_
