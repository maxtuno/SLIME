import satx

k = [114, 390, 579, 627, 633, 732, 921, 975]
e = 128

satx.engine(bits=3 * e, cnf_path='sum_of_three_cubes_still_open_large.cnf', simplify=True, signed=True)

x = satx.integer()
y = satx.integer()
z = satx.integer()

assert satx.one_of([x ** 3 - y ** 3 - z ** 3, x ** 3 + y ** 3 - z ** 3]) == satx.one_of(k)

if satx.satisfy(solver='./slime', log=True):
    print(x, y, z, x ** 3 - y ** 3 - z ** 3, x ** 3 + y ** 3 - z ** 3)
else:
    print('Infeasible...')