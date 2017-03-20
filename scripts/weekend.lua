


function scene_init()
	effects.fog(vec(.55, .5, 1.):from_hsv(), 180, 0.9, 0.1)
end

function leave_sky()
	effects.fog(vec(.55, .5, 1.):from_hsv(), 180, 0.9, 8.0)
end

function enter_sky()
	effects.fog(vec(.15, .3, 1.):from_hsv(), 280, 1.0, 8.0)
end