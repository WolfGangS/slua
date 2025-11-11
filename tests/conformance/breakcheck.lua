local m = setmetatable({}, {__index=breakcheck_index})

breakable()
-- The LOP_GETTABLE will happen before the call, which will trigger our "did break" check
nothing(m[1])
-- Same as above, but for LOP_NAMECALL
breakable()
m:foo(1)

return 'OK'
