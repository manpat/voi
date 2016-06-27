

local a = 1;
function fog(id)
	print(("Fog callback run %g by %d"):format(a, id))
	effects.fog(rand.gauss(vec(0.1), vec(math.sqrt(0.1))), rand.linear(50, 240), rand.linear(0.1, 0.7))
	-- effects.fog(vec(0.1), 50, 0.1)
	a = a + 1
end

effects.fog_interpolation(4)

print("Script run")
print("Vec:")
for k,v in pairs(vec) do print(k) end
print(vec())
print(vec(3))
print(vec(3, 5, 6))
print(vec(3, 5))
print(-vec(3, 5, 6))
print(vec(1,0,0) + vec(0,0,1))
print(vec(1,0,0) - vec(0,0,1))
print(vec(1,0,2) * vec(.5,0,1))
print(vec(1,2,3) / 2.)
print(vec.normalize(vec(1,0,1)))
print(vec.normalize(vec(3,0,0)))
print(vec.dot(vec(1,0,0), vec(0.5, 1, 0)))
print(vec.cross(vec(1,0,0), vec(0, 1, 0)))
print(vec.length(vec(1,0,1)))
local v = vec(1,2,3)
print(v:x(), v:y(), v:z())
v:x(3):y(2):z(1)
print(v:x(), v:y(), v:z())
print(v:dot(vec(0,1,0)))
print(v:normalize())

print("Rand:")
print(rand.spherical(1))
print(rand.linear(1, 2))
print(rand.linear(vec(1), vec(2)))
print(rand.gauss(1, math.sqrt(2)))
print(rand.gauss(vec(1), vec(math.sqrt(2))))
