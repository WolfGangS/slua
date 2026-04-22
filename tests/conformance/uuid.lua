local key = give_uuid()

local expected_str = "12345678-9abc-def0-1234-56789abcdef0"
local expected_key = uuid(expected_str)

assert(key == expected_key)
assert(tostring(key) == expected_str)
assert(#expected_key.bytes == 16)

local to_key = touuid(expected_str)
assert(to_key == key)

local expected_key_clone = uuid(buffer.fromstring(expected_key.bytes))

local expected_base64 = "EjRWeJq83vASNFZ4mrze8A"
local base64_key_string = uuid.tobase64(expected_key)
local key_from_base64 = uuid.frombase64(base64_key_string)
assert(expected_base64 == base64_key_string, "expected_base64: " .. expected_base64 .. " base64_key_string: " .. base64_key_string)
assert(key_from_base64 == expected_key, "from_key_base64: " .. tostring(key_from_base64) .. " expected_key: " .. tostring(expected_key))

assert(pcall(uuid.frombase64, "foo") == false)
assert(pcall(uuid.frombase64, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!") == false)
assert(pcall(uuid.frombase64, expected_key.bytes) == false)
-- Assert base64 is not reading the low 4 bits of the last byte
assert(uuid.frombase64("AAAAAAAAAAAAAAAAAAAAAA") == uuid.frombase64("AAAAAAAAAAAAAAAAAAAAAP"))


-- This should end up with the same UUID identity because of UUID interning.
assert(expected_key_clone == expected_key)

-- These two should be equal within tables
local tab = {}
tab[key] = 1
tab[expected_key] = 2

assert(tab[key] == 2)
assert(tab[expected_key] == 2)

-- with `uuid` conversion results are errors
assert(pcall(uuid, 'foo') == false)
-- with `touuid` we just return nil
assert(touuid('foo') == nil)
-- But the blank string is special-cased to mean `NULL_KEY`
assert(uuid('') == uuid('00000000-0000-0000-0000-000000000000'))

-- Invalid UUID in list cast should return NULL_KEY, not an uncompressed UUID
local null_key = uuid('00000000-0000-0000-0000-000000000000')
assert(ll.List2Key({"not-a-uuid"}, 1) == null_key)
assert(ll.List2Key({""}, 1) == null_key)
-- Valid UUID in list cast should work
assert(ll.List2Key({expected_str}, 1) == expected_key)

return "OK"
