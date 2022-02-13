// This file is covered by the LICENSE file in the root of this plugin.

#include "Weapon/TVRAmmoType.h"

UTVRAmmoType::UTVRAmmoType() : Super()
{
	bIsBuckShot = false;
	BuckShotCount = 1;
}

bool UTVRAmmoType::IsBuckShot() const
{
	return (bIsBuckShot && BuckShotCount > 1);
}
