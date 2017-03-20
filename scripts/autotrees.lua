


function scene_init()
	-- effects.fog(vec(.41, .8, 1.):from_hsv(), 60, 0.8, 0.1)
	effects.fog(vec(.4, .4, 0.1):from_hsv(), 50, 0.15, 0.1)

	local s = synth.new()

	local mod = s:sin(1/7 + s:sin(1/13)/9) * 0.3 + 0.6

	local o = s:tri(55 + s:sin(3/rand.linear(3, 7)) + s:sin(1/rand.linear(3, 7))) * (mod*0.6 + 0.4)
	o = o + (s:saw(30) * 0.3 + s:saw(31) * 0.4 + s:noise() * 0.05) * mod
	o = o + (s:sqr(25) * 0.2 + s:saw(620 + mod*20) * 0.1 + s:saw(650 + mod*2, 0.4) * 0.07) * (mod*0.7 + 0.3)
	o = s:lowpass(o, 600*mod^2 + 500)
	o = s:lowpass(o, 600*mod^2)

	s:output(o)
end