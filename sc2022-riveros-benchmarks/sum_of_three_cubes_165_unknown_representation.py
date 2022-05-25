import satx

k = 42
e = 80

satx.engine(bits=3 * e, cnf_path='sum_of_three_cubes_165_unknown_representation.cnf', simplify=True, signed=True)

x = satx.integer()
y = satx.integer()
z = satx.integer()

assert satx.one_of([x ** 3 - y ** 3 - z ** 3, x ** 3 + y ** 3 - z ** 3]) == k

assert x > 383344975542639445

if satx.satisfy(solver='./slime', log=True):
    print(x, y, z, x ** 3 - y ** 3 - z ** 3, x ** 3 + y ** 3 - z ** 3)
else:
    print('Infeasible...')