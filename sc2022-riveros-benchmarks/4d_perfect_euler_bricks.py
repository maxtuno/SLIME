import satx

satx.engine(bits=80, cnf_path='4d_perfect_euler_bricks.cnf', simplify=True, signed=True)

a = satx.integer()
b = satx.integer()
c = satx.integer()
d = satx.integer()
p = satx.integer()
q = satx.integer()
r = satx.integer()
s = satx.integer()
t = satx.integer()
u = satx.integer()
v = satx.integer()

satx.apply_single([a, b, c, d, p, q, r, s, t, u, v], lambda x: x > 0)

assert a ** 2 + b ** 2 == p ** 2
assert a ** 2 + c ** 2 == q ** 2
assert b ** 2 + c ** 2 == r ** 2
assert a ** 2 + d ** 2 == s ** 2
assert b ** 2 + d ** 2 == t ** 2
assert c ** 2 + d ** 2 == u ** 2
assert a ** 2 + b ** 2 + c ** 2 + d ** 2 == v ** 2

if satx.satisfy(solver='./slime', log=True):
    print(a, b, c, d, p, q, r, s, t, u, v)
else:
    print('Infeasible...')
