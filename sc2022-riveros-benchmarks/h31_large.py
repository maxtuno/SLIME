import satx

satx.engine(bits=128, cnf_path='h31_large.cnf', simplify=True, signed=True)

x = satx.integer()
y = satx.integer()
z = satx.integer()

assert y * (x ** 3 - y) == z ** 3 + 3

if satx.satisfy(solver='./slime', log=True):
    print(x, y, z)
else:
    print('Infeasible...')
