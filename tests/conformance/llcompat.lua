local foo = {1, 2, 3}

assert(ll.List2Float(foo, 1) == 1)
assert(ll.List2Integer(foo, 1) == 1)
assert(ll.List2Float(foo, -1) == 3)
-- Can't use 0 as an index at all, it will throw an error
assert(not pcall(ll.List2Float, foo, 0))

-- llcompat still uses 0-based indices
assert(llcompat.List2Float(foo, 0) == 1)
assert(llcompat.List2Float(foo, 1) == 2)
assert(llcompat.List2Float(foo, -1) == 3)
assert(llcompat.List2Integer(foo, 0) == 1)

return 'OK'
