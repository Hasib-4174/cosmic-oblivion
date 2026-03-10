#ifndef WEAPON_H
#define WEAPON_H

#include "constants.h"

void InitWeaponProjs(void);
void FireAdvancedWeapon(void);
void UpdateWeaponProjs(float dt);
void CheckWeaponCollisions(void);
void DrawWeaponEffects(void);

#endif
