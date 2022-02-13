// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/Component/TVRGunPrimaryHandSocket.h"

UTVRGunPrimaryHandSocket::UTVRGunPrimaryHandSocket(const FObjectInitializer& OI) : Super(OI)
{
	HandAnimTriggerPressed = nullptr;
	HandAnimTriggerSafe = nullptr;
	HandAnimTriggerTouch = nullptr;
}
