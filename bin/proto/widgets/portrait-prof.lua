name = "portrait-prof";
texture = "portrait-prof";
FunctionName = "CreateSprite";

z = 0.999;

animations = 
{
	{
		name = "left";
		frames =
		{
			{ dur = 100; num = 0 }
		}
	},
	{
		name = "right";
		frames =
		{
			{ com = constants.AnimComMirror },
			{ dur = 100; num = 0 }
		}
	}
}
