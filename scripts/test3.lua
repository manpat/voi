

local orb_triggers = {}

function init_orb_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local s = synth.new()
	local trg = s:trigger("frob_"..id)
	orb_triggers[id] = trg

	local bell = s:sin(55) * s:ar(0.01, 1, trg)^2 * 0.4
		+ s:tri(1760) * s:ar(0.001, 0.02, trg)^2 * 0.1

	s:output(bell)
	e:attach_synth(s)
end

local settings = {
	{vec(.5,.7, 1),110, 0.7, 2},
	{vec(.9,.6,.7),100, 0.6, 2},
	{vec(.2,.3,.6), 80, 0.2, 3},
	{vec(.3,.7,.8), 90, 0.4, 2},
	{vec(.5,.9,.7),100, 0.5, 2},
}

local l1 = 1

function next_fog(id)
	l1 = (l1%#settings)+1
	effects.fog(table.unpack(settings[l1]))

	local trg = orb_triggers[id]
	if trg then trg:trigger() end
end

function enter_layer1()
	color, dist, dens = table.unpack(settings[l1])
	effects.fog(color, dist, dens, 3)
end

function enter_layer2()
	effects.fog(vec(.5, 1.0, 0.7), 150, 0.8, 3)
end

function enter_layer3()
	effects.fog(vec(.9), 120, 0.4, 3)
end

function enter_layer4()
	effects.fog(vec(.1), 60, 0.4, 3)
end

local initial_fog_run = false
function initial_fog()
	if start_trigger_run then return end
	start_trigger_run = true

	color, dist, dens = table.unpack(settings[1])
	effects.fog(color, dist, dens, .01)
end

function animation_test()
	local e = entity.lookup("Platform")
	if not e then return end

	e:move_to(e:pos() + vec(0, e:size():y()+1, 0), 5)
end

local gate_open = false
local gate_start = nil
function open_gate(id)
	local e = entity.lookup("Gate")
	if not e then return end

	if not gate_start then
		gate_start = e:pos()
	end

	if gate_open then 
		e:move_to(gate_start, 5)
	else
		e:move_to(gate_start + vec(0, e:size():y()+1, 0), 5)
	end

	gate_open = not gate_open

	local trg = orb_triggers[id]
	if trg then trg:trigger() end
end

local gate_switch = false
function change_destination(id)
	local e = entity.lookup("Gate.Portal")
	if not e then return end

	if not gate_switch then
		e:set_layers(1, 3)
	else
		e:set_layers(1, 2)
	end

	gate_switch = not gate_switch

	local trg = orb_triggers[id]
	if trg then trg:trigger() end
end

function init_forest_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local pos = e:pos()
	local offset = math.sin((pos:x()/3 + pos:z() + rand.linear(-1, 1)) / 5 * math.pi * 2) * 0.5 + 0.5

	local s = synth.new()
	-- local osc = s:tri(55) + s:tri(55*3/2 + rand.linear(-1, 1)) 
	-- 	+ s:tri(110 + rand.linear(-1, 1)) 
	-- 	+ s:tri(330/2 + rand.linear(-2, 2))
	-- 	+ s:sin(332 + s:sin(6 + rand.linear(-1, 1))*3) * 0.5
	-- s:output(s:lowpass(osc, 120) * 0.3)
	local frequency = s:sin(1/8) * 1/7 + 1/6
	local mod = s:sin(frequency, offset) * 0.5 + 0.5

	local wind = s:highpass(s:noise(), 7200) * (mod * 0.9 + 0.1) * 0.05
	local ambience = s:lowpass(s:lowpass(s:noise()*2, 300 + s:sin(1/5 + s:sin(1/11)*1/7) * 100), 160) * 0.1

	s:output(wind + ambience)
	e:attach_synth(s)
end

function init_forest_gate_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local s = synth.new()
	local osc = s:noise()
	for i = 0,2 do
		osc = s:lowpass(osc*2, 80)
	end

	s:output(osc)
	e:attach_synth(s)
end

function init_chamber_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local s = synth.new()
	local osc = s:noise() + s:saw(35 + s:sin(rand.linear(1/3, 1/6))) + s:saw(42 + s:sin(rand.linear(1/3, 1/6)))*0.5 + s:sin(50)
	for i = 0,2 do
		osc = s:lowpass(osc*1.2, 160)
	end

	s:output(osc)
	e:attach_synth(s)
end

function init_catacombs_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local s = synth.new()
	local noise = s:noise() * (s:sin(s:sin(rand.linear(1/9, 1/6))*0.3 + rand.linear(0.3, 0.5)) * 0.2 + 0.8)
	for i = 0,2 do
		noise = s:lowpass(noise*2, 100)
	end

	local mod1 = (s:sin(1/9, rand.linear(0, 1)) * 0.3 + 0.5) * 0.3
	local mod2 = (s:sin(1/7, rand.linear(0, 1)) * 0.3 + 0.5) * 0.2
	local mod3 = (s:sin(1/5, rand.linear(0, 1)) * 0.3 + 0.5) * 0.05
	local mod4 = (s:sin(1/2, rand.linear(0, 1)) * 0.3 + 0.5) * 0.03
	local mod5 = (s:sin(1/2, rand.linear(0, 1)) * 0.3 + 0.5) * 0.02

	local osc = (s:tri(55) + s:tri(110)) * mod1
	osc = osc + (s:tri(110*3/2)) * mod2
	osc = osc + (s:sin(441)) * mod3
	osc = osc + (s:sin(440*3/2)) * mod4
	osc = osc + (s:sin(440*2)) * mod5

	s:output((s:lowpass(osc, 300) + noise) * 2)
	e:attach_synth(s)
end

function init_sea_sound(id)
	local e = entity.lookup(id)
	if not e then
		print("Couldn't find entity " .. id)
		return
	end

	local pos = e:pos()
	local offset = math.sin((pos:x() + pos:z()/2 + rand.linear(-2, 2)) / 20 * math.pi * 2) * 0.5 + 0.5

	local s = synth.new()
	local frequency = s:sin(1/21) * 1/10 + 1/11
	local mod = s:sin(frequency, offset) * 0.48 + 0.5

	local waves = s:highpass(s:noise(), 6000) * mod^2 * 0.3
		+ s:highpass(s:noise(), 300) * mod^2 * 0.5
		-- + s:noise() * (mod * 0.95 + 0.05)
		+ s:lowpass(s:noise(), 100) * (mod * 0.8 + 0.2)
		+ s:lowpass(s:noise()*2, 80 + mod*20) * (mod * 0.7 + 0.3)

	s:output(waves/2)
	e:attach_synth(s)
end