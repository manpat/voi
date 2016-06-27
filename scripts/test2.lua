

local a = 1;
function fog()
	print(("Fog callback run %g"):format(a))
	a = a + 1
end

print("Vec:")
for k,v in pairs(vec) do print(k) end
print(vec())
print(vec(3))
print(vec(3, 5, 6))
print(vec(3, 5))
print(vec(1,0,0) + vec(0,0,1))
print(vec(1,0,0) - vec(0,0,1))
print(vec(1,0,2) * vec(.5,0,1))
print(vec(1,2,3) / 2.)
print(vec.normalize(vec(1,0,1)))
print(vec.normalize(vec(3,0,0)))
print(vec.dot(vec(1,0,0), vec(0.5, 1, 0)))
print(vec.cross(vec(1,0,0), vec(0, 1, 0)))
print("Script run")