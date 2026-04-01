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

-- ll.ListReplaceList uses 1-based indices
local t = {"a", "b", "c", "d"}
assert(lljson.encode(ll.ListReplaceList(t, {"x"}, 2, 2)) == '["a","x","c","d"]')
assert(lljson.encode(ll.ListReplaceList(t, {"x", "y"}, 2, 3)) == '["a","x","y","d"]')
assert(lljson.encode(ll.ListReplaceList(t, {"x"}, 1, 1)) == '["x","b","c","d"]')
assert(lljson.encode(ll.ListReplaceList(t, {"x"}, -1, -1)) == '["a","b","c","x"]')
assert(lljson.encode(ll.ListReplaceList(t, {"x"}, 1, -1)) == '["x"]')

-- llcompat.ListReplaceList uses 0-based indices
assert(lljson.encode(llcompat.ListReplaceList(t, {"x"}, 1, 1)) == '["a","x","c","d"]')
assert(lljson.encode(llcompat.ListReplaceList(t, {"x"}, 0, 0)) == '["x","b","c","d"]')
assert(lljson.encode(llcompat.ListReplaceList(t, {"x"}, -1, -1)) == '["a","b","c","x"]')

return 'OK'
