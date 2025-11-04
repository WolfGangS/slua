-- Test that interrupt handlers are called before metamethods
-- The C side sets up an interrupt handler that clears a flag,
-- and metamethods that throw if the flag wasn't cleared.

-- Create a test object with test_metamethod as various metamethods
local function create_test_object()
    local obj = {}
    local mt = {
        __tostring = test_metamethod,
        __tojson = test_metamethod,
        __len = test_metamethod,
        __add = test_metamethod,
        __sub = test_metamethod,
        __mul = test_metamethod,
        __div = test_metamethod,
        __unm = test_metamethod,
        __index = test_metamethod,
        __newindex = test_metamethod,
    }
    return setmetatable(obj, mt)
end

-- Create test object once to avoid intermediate function calls
local obj = create_test_object()
local obj_len = {}
setmetatable(obj_len, {__len = test_metamethod})

-- Test __tostring (via luaL_callmeta path)
reset_interrupt_test()
local result = tostring(obj)
assert(result == "ok", "__tostring should return 'ok'")
assert(check_metamethod_ran(), "__tostring metamethod should have run")

-- Test __tojson (via JSON encoder)
reset_interrupt_test()
result = lljson.encode(obj)
assert(result == '"ok"', "__tojson should return '\"ok\"'")
assert(check_metamethod_ran(), "__tojson metamethod should have run")

-- Test __len (via JSON encoder when no __tojson)
reset_interrupt_test()
result = lljson.encode(obj_len)
assert(check_metamethod_ran(), "__len metamethod should have run")

-- Test arithmetic metamethods (__add, __sub, __mul, __div, __unm)
-- These go through callTMres or luaV_callTM paths
reset_interrupt_test()
local _ = obj + obj
assert(check_metamethod_ran(), "__add metamethod should have run")

reset_interrupt_test()
_ = obj - obj
assert(check_metamethod_ran(), "__sub metamethod should have run")

reset_interrupt_test()
_ = obj * obj
assert(check_metamethod_ran(), "__mul metamethod should have run")

reset_interrupt_test()
_ = obj / obj
assert(check_metamethod_ran(), "__div metamethod should have run")

reset_interrupt_test()
_ = -obj
assert(check_metamethod_ran(), "__unm metamethod should have run")

-- Test __index (via callTMres path)
reset_interrupt_test()
_ = obj.some_field
assert(check_metamethod_ran(), "__index metamethod should have run")

-- Test __newindex (via callTM path)
reset_interrupt_test()
obj.some_field = 123
assert(check_metamethod_ran(), "__newindex metamethod should have run")

return "OK"
