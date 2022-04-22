import satx 

satx.engine(bits=16, cnf_path='brocard_problem.cnf', simplify=True, signed=True)

n = satx.integer()
m = satx.integer()

n.is_not_in([0, 4, 5, 7])

assert satx.factorial(n) + 1 == m ** 2

if satx.satisfy(solver='./slime', log=True):
    print(n, m)
else:
    print('Infeasible...')