local wind_amt = nil

local mainHasRun = false
function main_update()
	if mainHasRun then return end
	mainHasRun = true

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

local hollowHasRun = false
function hollow_update()
	if not hollowHasRun then
		ent = entity.lookup("Monolith")
		ply = entity.lookup("Player")

		local s = synth.new()
		dist_param = s:value("dist")

		local dist2 = dist_param^2
		local mod = (s:sin(12 * dist2) * 0.5 + 0.5) * dist2
		local osc = s:lowpass(
			s:saw(dist2*4 + 35) 
			+ s:sqr(dist2*4 + mod + 21) 
			+ s:noise() * 0.8
			+ s:saw(55),
			10 + dist2*20)

		osc = s:lowpass(osc, 20 + dist2^2 *50) -- * dist2
		s:output(osc)

		ent:attach_synth(s)
	end

	hollowHasRun = true

	if ply and ent then
		local diff = ent:pos() - ply:pos()
		local dist = diff:length()
		local val = math.clamp(1-(dist-5)/40, 0, 1)

		if wind_amt then wind_amt:set(1-val*0.9, 0.1) end
		dist_param:set(val, 0.1)
	end
end

function math.clamp(v, mn, mx)
	return math.max(math.min(v, mx), mn)
end