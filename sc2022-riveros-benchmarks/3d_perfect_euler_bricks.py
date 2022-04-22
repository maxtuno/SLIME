import satx

satx.engine(bits=80, cnf_path='3d_perfect_euler_bricks.cnf', simplify=True, signed=True)

a = satx.integer()
b = satx.integer()
c = satx.integer()

p = satx.integer()
q = satx.integer()
r = satx.integer()
s = satx.integer()

satx.apply_single([a, b, c, p, q, r, s], lambda x: x > 0)

assert a ** 2 + b ** 2 == p ** 2
assert a ** 2 + c ** 2 == q ** 2
assert b ** 2 + c ** 2 == r ** 2
assert a ** 2 + b ** 2  + c ** 2 == s ** 2

if satx.satisfy(solver='./slime', log=True):
    print(a, b, c, p, q, r, s)
else:
    print('Infeasible...')
