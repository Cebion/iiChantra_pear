#ifndef __ANIMATION_FRAME_H_
#define  __ANIMATION_FRAME_H_


enum AnimationFrameCommand
{
	afcNone, afcRealX, afcRealY, afcRealW, afcRealH,
	afcInitW, afcInitH,
	afcShoot, afcShootX, afcShootY, afcShootDir, afcShootBeh,
	afcLoop, afcMenuLoop,
	afcSetDamageMult, afcStartDying,
	afcJumpIfCloseToCamera, afcJumpIfCloseToCameraLeft, afcJumpIfCloseToCameraRight, afcJumpIfCloseToCameraUp, afcJumpIfCloseToCameraDown,
	afcJumpIfObjectExists,
	afcSetAnim, afcSetAnimIfWeaponNotReady, afcSetAnimIfGunDirection,
	afcPushString, afcPushInt, afcPushRandomInt,
	afcJump, afcJumpIfYSpeedGreater, afcJumpIfXSpeedGreater, afcJumpIfWeaponReady, afcJumpRandom,
	afcJumpIfOnPlane, afcJumpIfIntEquals, afcLocalJumpIfIntEquals, afcLocalJumpIfNextIntEquals, afcJumpCheckFOV,
	afcJumpIfPlayerId,
	afcJumpIfXLess, afcJumpIfXGreater,
	afcRecover, afcMirror, afcWait,
	afcCreateObject, afcCreateEnemy, afcDestroyObject, afcCreateParticles, afcCreateItem, afcCreateEffect, afcSummonObject,
	afcMountPointSet, afcSetSolid,
	afcSetHealth, afcSetGravity, afcSetWaypoint,
	afcWaitForTarget, afcWaitForEnemy, afcMoveToTarget, afcMoveToTargetX,	afcMoveToTargetY, afcJumpIfTargetClose, afcFaceTarget, afcClearTarget,
	afcFlyToWaypoint, afcTeleportToWaypoint, afcSetNearestWaypoint,
	afcJumpIfTargetY, afcJumpIfTargetX, afcJumpIfTargetCloseByX, afcJumpIfTargetCloseByY, afcJumpIfStackIsNotEmpty,
	afcJumpIfWaypointClose,
	afcAdjustAim,
	afcCreateEnemyBullet, afcAimedShot, afcAngledShot, afcCreateEnemyRay, afcRandomAngledSpeed,
	afcPlaySound,
	afcJumpIfSquashCondition, afcBounceObject, afcSetAnimOnTargetPos, afcPushObject,
	afcGiveHealth, afcGiveAmmo, afcDamage, afcSetInvincible, afcReduceHealth, afcGiveWeapon,
	afcSetShielding, afcSetLifetime,
	afcDealDamage, afcDrop,
	afcAdjustX, afcAdjustY, afcSetRelativePos, afcAdjustHomingAcc,
	afcSetAccY, afcSetVelY, afcSetAccX, afcAdjustAccY, afcSetVelX, afcSetRelativeVelX, afcStop, afcSetMaxVelX, afcSetMaxVelY, afcSetZ,
	afcMapVarFieldAdd, afcEnemyClean,
	afcCallFunction, afcCallFunctionWithStackParameter,
	afcSetTouchable, afcSetInvisible, afcSetBulletCollidable,
	afcRandomOverlayColor, afcControlledOverlayColor, afcSetColor,
	afcPop, afcStopMorphing, afcSetShadow, afcReplaceWithRandomTile,
	afcEnvSound, afcEnvSprite, 
	afcBreakpoint
};

class AnimationFrame
{
public:
	UINT duration;
	UINT num;

	AnimationFrameCommand command;
	int param;
	char* txt_param;

	AnimationFrame()
	{
		duration = num  = param = 0;
		command = afcNone;
		txt_param = NULL;
	}

	AnimationFrame(const AnimationFrame& src);
	AnimationFrame& operator=(const AnimationFrame& src);

	~AnimationFrame()
	{
		DELETEARRAY(txt_param);
	}

};



#endif // __ANIMATION_FRAME_H_
