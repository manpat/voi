local wind_amt = nil

function main_init()
	local s = synth.new()
	wind_amt = s:value("wind_amt")

	local noisemod = s:sin(1/12 + s:sin(1/16)/16, math.pi*3/2) * 0.5 + 0.5
	local osc = s:lowpass(s:noise()*(noisemod*0.4 + 0.4), 15 + noisemod*100 + s:sqr(12*noisemod)*5*noisemod)
	s:output(osc * 0.4 * wind_amt)

	effects.fog(vec(4/12, 0.5, 1):from_hsv(), 110, 2., .01)
	-- effects.fog(vec(0.7, 0.7, 0.7):from_hsv(), 110, 0.3, .01)
end

local ent = nil
local ply = nil

local dist_param = nil

function monolith0_init()
	ent = entity.lookup("Monolith.0")
	ply = entity.lookup("Player")

	local s = synth.new()
	dist_param = s:value("dist")

	local dist2 = dist_param^2
	local mod = (s:sin(12 * dist2) * 0.5 + 0.5) * dist2
	local osc =
		s:saw(dist2*4 + 35) 
		+ s:sqr(dist2*4 + mod + 21) 
		+ s:noise() * 2
		+ s:saw(55)

	osc = s:lowpass(osc, 20 + dist2 *50)
	osc = s:lowpass(osc*0.8, 50 + dist2 *50)
	s:output(osc)

	ent:attach_synth(s)
end

function monolith0_update()
	if ply and ent then
		local diff = ent:pos() - ply:pos()
		local dist = diff:length()
		local val = math.clamp(1-(dist-5)/40, 0, 1)

		if wind_amt then wind_amt:set(1-val*0.9, 0.1) end
		dist_param:set(val, 0.1)
	end
end


local trg = nil
local dumb_time = 0
local trg_count = 0

function monolith1_init()
	local s = synth.new()
	trg = s:trigger("pulse")

	local env = s:ar(0.3, 3, trg) ^ 2

	local osc = s:sin(55)
		+ s:tri(45 + env*10)
		+ s:saw(220)
		+ s:sqr(221 + env*3) * 0.3
		+ s:noise() * 0.3

	osc = s:lowpass(osc, 50)
	osc = s:lowpass(osc, 50)
	s:output(osc * env)

	local ent = entity.lookup("Monolith.1")
	ent:attach_synth(s)
end

function monolith1_update(id, dt)
	dumb_time = dumb_time + dt / 3
	if math.floor(dumb_time) > trg_count then
		trg:trigger()
		trg_count = math.floor(dumb_time)
	end
end

function math.clamp(v, mn, mx)
	return math.max(math.min(v, mx), mn)
end