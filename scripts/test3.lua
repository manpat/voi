
local settings = {
	{vec(.5,.7,1), 110, 0.6, 1},
	{vec(.2,.3,.6), 80, 0.4, 1},
	{vec(.3,.1,.1), 50, 0.2, 1},
	{vec(.8,.5,.3), 90, 0.4, 1},
	{vec(.6,.3,.6), 90, 0.4, 1},
	{vec(.4,.9,.6),100, 0.5, 1},
}

local n = 1

function start_trigger()
	effects.fog(table.unpack(settings[n]))
	n = (n%#settings)+1
end