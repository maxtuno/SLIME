# SLIME SAT Solver

### A Free Massive World Class High Performance SAT Solver.

    c
    c   ██████  ██▓     ██▓ ███▄ ▄███▓▓█████
    c ▒██    ▒ ▓██▒    ▓██▒▓██▒▀█▀ ██▒▓█   ▀
    c ░ ▓██▄   ▒██░    ▒██▒▓██    ▓██░▒███
    c   ▒   ██▒▒██░    ░██░▒██    ▒██ ▒▓█  ▄
    c ▒██████▒▒░██████▒░██░▒██▒   ░██▒░▒████▒
    c ▒ ▒▓▒ ▒ ░░ ▒░▓  ░░▓  ░ ▒░   ░  ░░░ ▒░ ░
    c ░ ░▒  ░ ░░ ░ ▒  ░ ▒ ░░  ░      ░ ░ ░  ░
    c ░  ░  ░    ░ ░    ▒ ░░      ░      ░
    c       ░      ░  ░ ░         ░      ░  ░
    c
    c                  [CORE]
    c

### Winner of Crypto Track and 3rd place on Cloud Track at the SAT Competition 2021 https://satcompetition.github.io/2021/
<img src="https://raw.githubusercontent.com/maxtuno/SLIME/master/medals.jpeg"/>

### Cryptography
  This version of SLIME solve 80% of Crypto Track 2021 at 18000 seconds.
  
    usage: 
    slime -use-distance -cryptography cnf.instance
    slime -use-distance -cryptography cnf.instance cnf.proof
    slime -use-distance -cryptography cnf.instance cnf.proof cnf.model
  
### General SAT
    usage:
    slime cnf.instance
    slime cnf.instance cnf.proof
    slime cnf.instance cnf.proof cnf.model
    
### Research Papers Citing SLIME

Certifying Parity Reasoning Efficiently Using Pseudo-Boolean Proofs https://arxiv.org/abs/2209.12185

Power-aware test scheduling framework for IEEE 1687 multi-power domain networks using formal techniques https://www.sciencedirect.com/science/article/abs/pii/S0026271422000750

SAT Solving in the Serverless Cloud https://par.nsf.gov/servlets/purl/10319044
