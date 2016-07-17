
local settings = {
	{vec(.5,.7, 1),110, 0.7, 2},
	{vec(.9,.6,.7),100, 0.6, 2},
	{vec(.2,.3,.6), 80, 0.2, 3},
	{vec(.3,.7,.8), 90, 0.4, 2},
	{vec(.5,.9,.7),100, 0.5, 2},
}

local l1 = 1

function next_fog()
	l1 = (l1%#settings)+1
	effects.fog(table.unpack(settings[l1]))
end

function enter_layer1()
	color, dist, dens = table.unpack(settings[l1])
	effects.fog(color, dist, dens, 3)
end

function enter_layer2()
	effects.fog(vec(.5,1.,.7), 150, 0.8, 3)
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
function open_gate()
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
end

function change_destination(id)
	local e = entity.lookup("Gate.Portal")
	if not e then return end

	print("Gate layers:", e:layers())
	e:set_layers(e:layers())
end